#pragma once

#include <Arduino.h>
#include <stubborn_sender.h>

#include "FHSS.h"
#include "SX1280Driver.h"
#include "common.h"
#include "uECDH.h"

#define DATA_SIZE 32

extern HardwareSerial DebugSerial;

class TxHandshakeClass {
 public:
  enum handshake_state_t {
    INIT = 0,
    WAIT_HELLO,
    RECV_HELLO,
    SEND_HELLO,
    SEND_ECDH_PUB_KEY,
    WAIT_ECDH_PUB_KEY,
    RECV_ECDH_PUB_KEY,
    SEND_BYE,
    DONE
  };

  TxHandshakeClass() { }

  void Init() {
    tx_handshake_state_ = INIT;
    busy_transmitting_ = false;
    transmitting_radio_ = Radio.GetLastSuccessfulPacketRadio();

    txEcdh.init();
    txEcdh.compress_public_key(pub_key_, &pub_key_len_);
  }

  inline bool IsDone() { return tx_handshake_state_ == DONE; };

  void DoHandle() {
    int ret = 1;
    size_t i = 0;
    static long timeout = 0;
    static long handle_timeout = 0;

    switch (tx_handshake_state_) {
      case INIT:
        tx_handshake_state_ = WAIT_HELLO;
        timeout = millis() + 50;  // wait for 50ms seconds
        handle_timeout = millis() + 10000;  // wait for 10 seconds
        Radio.RXnb();
        break;
      case WAIT_HELLO:
        if (millis() > timeout) {
            DebugSerial.println("TX: Timeout waiting for hello, restarting handshake");
            tx_handshake_state_ = INIT;
        }
        break;
      case RECV_HELLO:
        tx_handshake_state_ = SEND_HELLO;
        break;
      case SEND_HELLO:
        DebugSerial.println("send hello");
        handshake_hello();
        while (Busy()) {}
        tx_handshake_state_ = SEND_ECDH_PUB_KEY;
        break;
      case SEND_ECDH_PUB_KEY:
        DebugSerial.println("send ecdh pub key");
        for (int i = 0; i < 32; i++) {
          DebugSerial.printf("%02x", pub_key_[i]);
        }
        DebugSerial.println();

        handshake_send_ecdh_pub_key();
        while (Busy()) {}
        tx_handshake_state_ = WAIT_ECDH_PUB_KEY;
        timeout = millis() + 100;  // wait for 0.1 seconds
        Radio.RXnb();
        break;
      case WAIT_ECDH_PUB_KEY:
        if (millis() > timeout) {
          DebugSerial.println("TX: Timeout waiting for ECDH public key, restarting handshake");
          tx_handshake_state_ = SEND_ECDH_PUB_KEY;
        }

       break;
      case RECV_ECDH_PUB_KEY:
        DebugSerial.println("recv ecdh pub key");
        for (i = 0; i < 32; i++) {
          DebugSerial.printf("%02x", Radio.RXdataBuffer[i]);
        }
        DebugSerial.println();
        memcpy(rx_compressed_public_key_, Radio.RXdataBuffer, 32);
        tx_handshake_state_ = SEND_BYE;
        break;
      case SEND_BYE:
        DebugSerial.println("send bye");
        // send 3 bye messages to ensure the receiver gets it
        handshake_bye();
        while (Busy()) {}
        handshake_bye();
        while (Busy()) {}
        handshake_bye();
        while (Busy()) {}
        tx_handshake_state_ = DONE;
        break;
      case DONE:
        digitalWrite(GPIO_PIN_LED, !digitalRead(GPIO_PIN_LED));
        tx_handshake_state_ = INIT;
        break;
    }
    if (millis() > handle_timeout) {
      DebugSerial.println("RX: Timeout waiting for handshake, restarting");
      tx_handshake_state_ = INIT;
    }
  };

  void TXdoneCallback() { Busy(false); };

  bool RXdoneCallback(SX12xxDriverCommon::rx_status const status) {
    DebugSerial.println("->RXdoneCallback");
    if (State() == WAIT_HELLO) {
      HandleWaitHello();
      return true;
    }

    if (State() == WAIT_ECDH_PUB_KEY) {
      HandleWaitEcdhPubKey();
      return true;
    }
  };

  void HandleWaitHello() {
    if (memcmp(Radio.RXdataBuffer, "hello", 5) == 0) {
      DebugSerial.println("got hello");
      tx_handshake_state_ = RECV_HELLO;
      return;
    }

    Radio.RXnb();
  };

  void HandleWaitEcdhPubKey() {
    // if (Radio.RXdataBuffer[0] == 0xEC) {
     DebugSerial.println("got ecdh pub key");
     tx_handshake_state_ = RECV_ECDH_PUB_KEY;
     return;
    // }

    // Radio.RXnb();
  };

  int LeaKey(uint8_t *K, size_t &K_len, uint8_t *A, size_t &A_len, uint8_t *N,
             size_t &N_len) {
    if (State() != DONE) return -1;
    memcpy(K, K_, 16);
    memcpy(A, A_, 16);
    memcpy(N, N_, 16);
    K_len = 16;
    A_len = 16;
    N_len = 16;

    txEcdh.generate_secret_key((const char *)rx_compressed_public_key_, rx_compressed_public_key_len_);
    txEcdh.export_secret_key(tx_secret_key, &tx_secret_key_len);
    DebugSerial.print("\r\nTX secret key: ");
    for (size_t i = 0; i < 32; i++) {
      DebugSerial.printf("%02x", tx_secret_key[i]);
    }
    DebugSerial.println();

    return 0;
  }

 private:
  inline void Busy(bool busyTransmitting) {
    busy_transmitting_ = busyTransmitting;
  };

  handshake_state_t State() { return tx_handshake_state_; };

  inline bool Busy() { return busy_transmitting_; };

  void handshake_hello() {
    Busy(true);
    Radio.TXnb((uint8_t *)"hello", 5, transmitting_radio_);
  };

  void handshake_send_ecdh_pub_key() {
    // unsigned char buffer_[DATA_SIZE] = {0xEC, 0xD0, 0x0, };
    Busy(true);
    // buffer_[2] = pub_key_len_;
    // memcpy(buffer_ + 3, pub_key_, pub_key_len_);
    // Radio.TXnb(buffer_, sizeof(buffer_), transmitting_radio_);
    // Radio.TXnb(pub_key_, pub_key_len_, transmitting_radio_);
    Radio.TXnb(pub_key_, 32, transmitting_radio_);
  };

  void handshake_bye() {
    Busy(true);
    Radio.TXnb((uint8_t *)"bye", 3, transmitting_radio_);
  };


  handshake_state_t tx_handshake_state_;
  bool busy_transmitting_;

  // generate plaintext for test
  uint8_t K_[16] = {0x14, 0x87, 0x0B, 0x99, 0x92, 0xEA, 0x89, 0x67, 0x8A, 0x1D, 0xDF, 0xD6, 0x30, 0x91, 0x8D, 0xF0};
  uint8_t A_[16] = {0};
  uint8_t N_[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };


  // ecdh
  unsigned char pub_key_[64] = {0};
  size_t pub_key_len_ = 0;
  unsigned char rx_compressed_public_key_[64] = {0};
  size_t rx_compressed_public_key_len_ = 0;
  int pub_key_msg_num_ = 0;

  uint8_t tx_secret_key[32];
  size_t tx_secret_key_len = 0;

  ECDH txEcdh;

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
