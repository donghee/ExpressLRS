#pragma once

#include <Arduino.h>
#include <stubborn_sender.h>

#include "FHSS.h"
#include "SX1280Driver.h"
#include "common.h"
#include "rsa.h"

#define DATA_SIZE 32

class TxHandshakeClass {
 public:
  enum handshake_state_t {
    INIT = 0,
    HELLO,
    SEND_RSA_PUB_KEY,
    WAIT_ACK,
    RECV_LEA_KEY,
    DECRYPT_LEA_KEY,
    DONE
  };

  TxHandshakeClass() { Init(); }

  void Init() {
    tx_handshake_state_ = INIT;
    busy_transmitting_ = false;
    transmitting_radio_ = Radio.GetLastSuccessfulPacketRadio();

    GeneratePubKey();
  }

  inline bool IsDone() { return tx_handshake_state_ == DONE; };

  void DoHandle() {
    int ret = 1;
    size_t i = 0;

    switch (tx_handshake_state_) {
      case INIT:
        tx_handshake_state_ = HELLO;
        break;

      case HELLO:
        handshake_hello();

        while (Busy()) {
        }

        Busy(true);
        tx_handshake_state_ = SEND_RSA_PUB_KEY;
        sender_.setMaxPackageIndex(ELRS4_TELEMETRY_MAX_PACKAGES);
        sender_.ResetState();
        sender_.SetDataToTransmit(pub_key_, 32);
        package_index_ = prepareDataForPubKey();  // TODO, add data reference
        pub_key_msg_seq_ = 0;

        Radio.TXnb(data, sizeof(data), transmitting_radio_);
        break;

      case SEND_RSA_PUB_KEY:
        if (!Busy()) {
          ret = handshake_send_rsa_pub_key();
        }
        if (ret == 0) {
          pub_key_timeout_ = millis() + 10;
          tx_handshake_state_ = WAIT_ACK;
          Radio.RXnb();
        }
        break;

      case WAIT_ACK:
        if (millis() > pub_key_timeout_) {  // on timeout, restart handshake
          tx_handshake_state_ = INIT;
        }
        break;

      case RECV_LEA_KEY:
        // No operation defined for this case
        break;

      case DECRYPT_LEA_KEY:
        ret = rsa_.decrypt(lea_key_, decryptedtext_, &i, 1024);
        if (ret != 0) DBGLN("FAILED DECRYPT");

        if (ret == 0) {
          // memcpy(K_, decryptedtext, 16); memcpy(A_, decryptedtext + 16, 16);
          // memcpy(N_, decryptedtext + 32, 12);
          tx_handshake_state_ = DONE;
        } else {
          tx_handshake_state_ = INIT;
        }
        break;

      case DONE:
        digitalWrite(GPIO_PIN_LED, !digitalRead(GPIO_PIN_LED));
        tx_handshake_state_ = INIT;
        break;
    }
  };

  void TXdoneCallback() { Busy(false); };

  bool RXdoneCallback(SX12xxDriverCommon::rx_status const status) {
    // to handle multiple ack msg, ignore handshake state.
    if (memcmp(Radio.RXdataBuffer, "ack", 3) == 0) {
      HandleWaitAck();
      return true;
    }

    if (State() == RECV_LEA_KEY) {
      HandleRecvLeaKey();
      return true;
    }
  };

  int LeaKey(uint8_t *K, size_t &K_len, uint8_t *A, size_t &A_len, uint8_t *N,
             size_t &N_len) {
    if (State() != DONE) return -1;
    memcpy(K, decryptedtext_, 16);
    memcpy(A, decryptedtext_ + 16, 16);
    memcpy(N, decryptedtext_ + 32, 12);
    K_len = 16;
    A_len = 16;
    N_len = 12;

    return 0;
  }

 private:
  inline void Busy(bool busyTransmitting) {
    busy_transmitting_ = busyTransmitting;
  };

  handshake_state_t State() { return tx_handshake_state_; };

  inline bool Busy() { return busy_transmitting_; };

  void GeneratePubKey() {
    int ret = 1;
    const char *pers = "rsa_genkey";

    // generate public key
    ret = rsa_.generate_key(pers, strlen(pers));
    if (ret != 0) DBGLN("FAILED GENERATE KEY");
    ret = rsa_.export_pubkey(pub_key_, &pub_key_len_);
    if (ret != 0) DBGLN("FAILED EXPORT PUB KEY");
  };

  void HandleWaitAck() {
    tx_handshake_state_ = RECV_LEA_KEY;
    lea_key_package_index_ = 0;
    lea_key_len_ = 0;
    Radio.RXnb();
  };

  void HandleRecvLeaKey() {
    if (lea_key_package_index_ == 15) {
      memcpy(lea_key_ + (lea_key_package_index_ * 8), Radio.RXdataBuffer, 8);
      tx_handshake_state_ = DECRYPT_LEA_KEY;
      return;
    }

    memcpy(lea_key_ + (lea_key_package_index_ * 8), Radio.RXdataBuffer, 8);
    lea_key_package_index_++;
    lea_key_len_ += 8;
    Radio.RXnb();
  };

  void handshake_hello() {
    Busy(true);
    Radio.TXnb((uint8_t *)"hello", 5, transmitting_radio_);
  };

  uint8_t prepareDataForPubKey() {
    uint8_t packageIndex_;
    packageIndex_ = sender_.GetCurrentPayload(data, sizeof(data));
    sender_.ConfirmCurrentPayload(confirm_value_);
    confirm_value_ = !confirm_value_;
    return packageIndex_;
  };

  int handshake_send_rsa_pub_key() {
    Busy(true);

    if (pub_key_msg_seq_ == 8) {  // last pub key msg
      pub_key_msg_seq_ = 0;
      return 0;  // done sending pub key
    }

    package_index_ = prepareDataForPubKey();

    if (package_index_ == 0) {  // at last package of current msg, prepare next
                                // 32 bytes pub key msg
      sender_.setMaxPackageIndex(ELRS4_TELEMETRY_MAX_PACKAGES);
      sender_.ResetState();
      sender_.SetDataToTransmit(pub_key_ + (32 * (pub_key_msg_seq_ + 1)), 32);
      pub_key_msg_seq_++;
    }

    Radio.TXnb(data, sizeof(data), transmitting_radio_);

    return 1;  // continue sending pub key
  };

  handshake_state_t tx_handshake_state_;
  bool busy_transmitting_;

  // encrypted lea key from RX
  unsigned char lea_key_[512] = {0};
  size_t lea_key_len_ = 0;
  int lea_key_package_index_ = 0;

  // decrypted text which lea key from encrypted lea key
  unsigned char decryptedtext_[1024] = {0};
  // uint8_t K_[16] = {0};
  // uint8_t A_[16] = {0};
  // uint8_t N_[12] = {0};

  // rsa
  unsigned char pub_key_[1024] = {0};
  size_t pub_key_len_ = 0;
  int pub_key_msg_num_ = 0;
  RSA rsa_;

  // handshake
  StubbornSender sender_;
  volatile uint8_t package_index_;
  volatile int pub_key_msg_seq_ = 0;
  uint8_t data[8];
  volatile bool confirm_value_ = true;
  uint32_t pub_key_timeout_ = 10;
  uint32_t lea_key_timeout_ = 10;

  SX12XX_Radio_Number_t transmitting_radio_;
};
