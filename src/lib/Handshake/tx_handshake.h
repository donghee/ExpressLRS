#pragma once

#include <Arduino.h>
#include "SX1280Driver.h"
#include "common.h"
#include "FHSS.h"
#include "rsa.h"
#include <stubborn_sender.h>

#define DATA_SIZE 32

enum tx_handshake_state_t {
  HANDSHAKE_INIT,
  HANDSHAKE_HELLO,
  HANDSHAKE_SEND_RSA_PUB_KEY,
  HANDSHAKE_WAIT_ACK,
  HANDSHAKE_RECV_LEA_KEY,
  HANDSHAKE_DECRYPT_LEA_KEY,
  HANDSHAKE_DONE
};

class TxHandshakeClass {
 public:

  TxHandshakeClass() {
    init();
  }

  void init() {
    tx_handshake_state = HANDSHAKE_INIT;
    busyTransmitting = false;
    transmittingRadio = Radio.GetLastSuccessfulPacketRadio();

    generate_pub_key();
  }

  tx_handshake_state_t state() { return tx_handshake_state; };
  inline bool done() { return tx_handshake_state == HANDSHAKE_DONE; };

  inline bool busy() { return busyTransmitting; };
  inline void busy(bool busyTransmitting_) { busyTransmitting = busyTransmitting_; };

  void do_handle() {
    int ret = 1;
    size_t i = 0;

    switch (tx_handshake_state) {
      case HANDSHAKE_INIT:
        tx_handshake_state = HANDSHAKE_HELLO;
        break;

      case HANDSHAKE_HELLO:
        handshake_hello();

        while (busy()) { }

        busy(true);
        tx_handshake_state = HANDSHAKE_SEND_RSA_PUB_KEY;
        sender.setMaxPackageIndex(ELRS4_TELEMETRY_MAX_PACKAGES);
        sender.ResetState();
        sender.SetDataToTransmit(pub_key, 32);
        packageIndex = prepareDataForPubKey(); // TODO, add data reference
        pub_key_msg_seq = 0;

        Radio.TXnb(data, sizeof(data), transmittingRadio);
        break;

      case HANDSHAKE_SEND_RSA_PUB_KEY:
        if (!busy()) {
          ret = handshake_send_rsa_pub_key();
        }
        if (ret == 0) {
          pub_key_timeout = millis() + 10;
          tx_handshake_state = HANDSHAKE_WAIT_ACK;
          Radio.RXnb();
        }
        break;

      case HANDSHAKE_WAIT_ACK:
        if (millis() > pub_key_timeout) {  // on timeout, restart handshake
          tx_handshake_state = HANDSHAKE_INIT;
        }
        break;

      case HANDSHAKE_RECV_LEA_KEY:
        // No operation defined for this case
        break;

      case HANDSHAKE_DECRYPT_LEA_KEY:
        ret = rsa.decrypt(lea_key, decryptedtext, &i, 1024);
        if (ret != 0)
          DBGLN("FAILED DECRYPT");

        if (ret == 0) {
          memcpy(K, decryptedtext, 16); memcpy(A, decryptedtext + 16, 16); memcpy(N, decryptedtext + 32, 12);
          tx_handshake_state = HANDSHAKE_DONE;
        } else {
          tx_handshake_state = HANDSHAKE_INIT;
        }
        break;

      case HANDSHAKE_DONE:
        digitalWrite(GPIO_PIN_LED, !digitalRead(GPIO_PIN_LED));
        tx_handshake_state = HANDSHAKE_INIT;
        break;
    }
  };

  void handle_wait_ack()
  {
    tx_handshake_state = HANDSHAKE_RECV_LEA_KEY;
    lea_key_packageIndex = 0;
    lea_key_len = 0;
    Radio.RXnb();
  };

  void handle_recv_lea_key()
  {
    if (lea_key_packageIndex == 15) {
      memcpy(lea_key + (lea_key_packageIndex * 8), Radio.RXdataBuffer, 8);
      tx_handshake_state = HANDSHAKE_DECRYPT_LEA_KEY;
      return;
    }

    memcpy(lea_key + (lea_key_packageIndex * 8), Radio.RXdataBuffer, 8);
    lea_key_packageIndex++;
    lea_key_len += 8;
    Radio.RXnb();
  };

  void handshake_hello()
  {
    busy(true);
    Radio.TXnb((uint8_t*)"hello", 5, transmittingRadio);
  };

  uint8_t prepareDataForPubKey()
  {
    uint8_t packageIndex_;
    packageIndex_ = sender.GetCurrentPayload(data, sizeof(data));
    sender.ConfirmCurrentPayload(confirmValue);
    confirmValue = !confirmValue;
    return packageIndex_;
  };

  int handshake_send_rsa_pub_key()
  {
    busy(true);

    if (pub_key_msg_seq == 8) { // last pub key msg
      pub_key_msg_seq = 0;
      return 0; // done sending pub key
    }

    packageIndex = prepareDataForPubKey();

    if (packageIndex == 0) { // at last package of current msg, prepare next 32 bytes pub key msg
      sender.setMaxPackageIndex(ELRS4_TELEMETRY_MAX_PACKAGES);
      sender.ResetState();
      sender.SetDataToTransmit(pub_key + (32 * (pub_key_msg_seq + 1)), 32);
      pub_key_msg_seq++;
    }

    Radio.TXnb(data, sizeof(data), transmittingRadio);

    return 1; // continue sending pub key
  };

  void TXdoneCallback()
  {
    busy(false);
  };

  bool RXdoneCallback(SX12xxDriverCommon::rx_status const status)
  {
    // to handle multiple ack msg, ignore handshake state.
    if (memcmp(Radio.RXdataBuffer, "ack", 3) == 0) {
      handle_wait_ack();
      return true;
    }

    if (state() == HANDSHAKE_RECV_LEA_KEY) {
      handle_recv_lea_key();
      return true;
    }
  };

 private:
  void generate_pub_key()
  {
    int ret = 1;
    const char *pers = "rsa_genkey";

    // generate public key
    ret = rsa.generate_key(pers, strlen(pers));
    if ( ret != 0 )
      DBGLN("FAILED GENERATE KEY");
    ret = rsa.export_pubkey(pub_key, &pub_key_len);
    if ( ret != 0 )
      DBGLN("FAILED EXPORT PUB KEY");
  };

  tx_handshake_state_t tx_handshake_state;
  bool busyTransmitting;

  // encrypted lea key
  unsigned char lea_key[512] = {0};
  size_t lea_key_len = 0;
  int lea_key_packageIndex = 0;

  // decrypt lea key
  unsigned char decryptedtext[1024] = {0};
  uint8_t K[16] = {0};
  uint8_t A[16] = {0};
  uint8_t N[12] = {0};

  // rsa
  unsigned char pub_key[1024] = {0};
  size_t pub_key_len = 0;
  int pub_key_msg_num = 0;
  RSA rsa;

  // handshake
  StubbornSender sender;
  volatile uint8_t packageIndex;
  volatile int pub_key_msg_seq = 0;
  uint8_t data[8];
  volatile bool confirmValue = true;
  uint32_t pub_key_timeout = 10;
  uint32_t lea_key_timeout = 10;

  SX12XX_Radio_Number_t transmittingRadio;
};
