#pragma once

#include <Arduino.h>
#include <stubborn_sender.h>

#include "FHSS.h"
#include "SX1280Driver.h"
#include "common.h"
#include "uECDH.h"

#define DATA_SIZE 32

extern HardwareSerial DebugSerial;

class RxHandshakeClass {
 public:
  enum handshake_state_t {
    INIT = 0,
    SEND_HELLO,
    WAIT_HELLO,
    RECV_HELLO,
    WAIT_ECDH_PUB_KEY,
    RECV_ECDH_PUB_KEY,
    SEND_ECDH_PUB_KEY,
    WAIT_BYE,
    DONE
  };

  RxHandshakeClass() { }

  void Init() {
    rx_handshake_state_ = INIT;
    busy_transmitting_ = false;
    transmitting_radio_ = Radio.GetLastSuccessfulPacketRadio();

    rxEcdh.init();
    rxEcdh.compress_public_key(pub_key_, &pub_key_len_);
  }

  inline bool IsDone() { return rx_handshake_state_ == DONE; };

  void DoHandle() {
    int ret = 1;
    static long timeout = 0;
    static long handle_timeout = 0;

    switch (rx_handshake_state_) {
      case INIT:
        rx_handshake_state_ = SEND_HELLO;
        handle_timeout = millis() + 10000;  // wait for 10 seconds
        break;
      case SEND_HELLO:
        DBGLN("send hello");
        handshake_hello();
        while (Busy()) {}
        rx_handshake_state_ = WAIT_HELLO;
        timeout = millis() + 50;  // wait for 50ms seconds
        Radio.RXnb();
        break;
      case WAIT_HELLO:
        if (millis() > timeout) {
          DBGLN("RX: Timeout waiting for hello, restarting handshake");
          rx_handshake_state_ = INIT;
        }
        break;
      case RECV_HELLO:
        rx_handshake_state_ = WAIT_ECDH_PUB_KEY;
        timeout = millis() + 50;  // wait for 0.1 seconds
        Radio.RXnb();
        break;
      case WAIT_ECDH_PUB_KEY:
        if (millis() > timeout) {
          DBGLN("RX: Timeout waiting for ECDH public key, restarting handshake");
          rx_handshake_state_ = RECV_HELLO;
        }
        break;
      case RECV_ECDH_PUB_KEY:
        DBGLN("recv ecdh pub key");
        for (int i = 0; i < 32; i++) {
          DebugSerial.printf("%02x", Radio.RXdataBuffer[i]);
        }
        DebugSerial.println();
        memcpy(tx_compressed_public_key_, Radio.RXdataBuffer, 32);
        rx_handshake_state_ = SEND_ECDH_PUB_KEY;
        break;
      case SEND_ECDH_PUB_KEY:
        DBGLN("send ecdh pub key");
        for (int i = 0; i < 32; i++) {
          DebugSerial.printf("%02x", pub_key_[i]);
        }
        DebugSerial.println();

        handshake_send_ecdh_pub_key();
        while (Busy()) {}
        rx_handshake_state_ = WAIT_BYE;
        timeout = millis() + 100;  // wait for 0.1 seconds
        Radio.RXnb();
        break;
      case WAIT_BYE:
        if (millis() > timeout) {
          DBGLN("RX: Timeout waiting for bye, restarting handshake");
          rx_handshake_state_ = SEND_ECDH_PUB_KEY;
        }
       break;
      case DONE:
        digitalWrite(GPIO_PIN_LED, !digitalRead(GPIO_PIN_LED));
        rx_handshake_state_ = INIT;
        break;
    }
    if (millis() > handle_timeout) {
      DBGLN("RX: Timeout waiting for handshake, restarting");
      rx_handshake_state_ = INIT;
    }
  };

  void TXdoneCallback() { Busy(false); };

  bool RXdoneCallback(SX12xxDriverCommon::rx_status const status) {
    DBGLN("->RXdoneCallback");
    if (State() == WAIT_HELLO) {
      HandleWaitHello();
      return true;
    }
    if (State() == WAIT_ECDH_PUB_KEY) {
      HandleWaitEcdhPubKey();
      return true;
    }
    if (State() == WAIT_BYE) {
      HandleWaitBye();
      return true;
    }
  };

  int LeaKey(uint8_t *K, size_t &K_len, uint8_t *A, size_t &A_len, uint8_t *N,
             size_t &N_len) {
    if (State() != DONE) return -1;

    rxEcdh.generate_secret_key((const char *)tx_compressed_public_key_, tx_compressed_public_key_len_);
    rxEcdh.export_secret_key(rx_secret_key, &rx_secret_key_len);
    DebugSerial.print("\r\nRX secret key: ");
    for (size_t i = 0; i < 32; i++) {
      DebugSerial.printf("%02x", rx_secret_key[i]);
    }
    DebugSerial.println("\r\n");

    memcpy(K_, rx_secret_key, 16);
    memcpy(A_, rx_secret_key + 16, 16);
    memcpy(N_, rx_secret_key + 8, 16);

    memcpy(K, K_, 16);
    memcpy(A, A_, 16);
    memcpy(N, N_, 16);
    K_len = 16;
    A_len = 16;
    N_len = 16;

    return 0;
  }

 private:
  inline void Busy(bool busyTransmitting_) {
    busy_transmitting_ = busyTransmitting_;
  };

  handshake_state_t State() { return rx_handshake_state_; };

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

  void HandleWaitHello() {
    if (memcmp(Radio.RXdataBuffer, "hello", 5) == 0) {
      DBGLN("got hello");
      rx_handshake_state_ = RECV_HELLO;
      return;
    }

    Radio.RXnb();
  };

  void HandleWaitEcdhPubKey() {
    // if (Radio.RXdataBuffer[0] == 0xEC) {
      DBGLN("got ecdh pub key");
      rx_handshake_state_ = RECV_ECDH_PUB_KEY;
      return;
    // }

    // Radio.RXnb();
  };

  void HandleWaitBye() {
    if (memcmp(Radio.RXdataBuffer, "bye", 3) == 0) {
      DBGLN("got bye");
      rx_handshake_state_ = DONE;
      return;
    }

    Radio.RXnb();
  };


  handshake_state_t rx_handshake_state_;
  bool busy_transmitting_;

  int ack_ = 0;

  // generate plaintext for test
  uint8_t K_[16] = {0x14, 0x87, 0x0B, 0x99, 0x92, 0xEA, 0x89, 0x67, 0x8A, 0x1D, 0xDF, 0xD6, 0x30, 0x91, 0x8D, 0xF0};
  uint8_t A_[16] = {0};
  uint8_t N_[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };

  // ecdh
  unsigned char pub_key_[64] = {};
  size_t pub_key_len_ = 0;
  unsigned char tx_compressed_public_key_[64] = {0};
  size_t tx_compressed_public_key_len_ = 0;

  uint8_t rx_secret_key[32];
  size_t rx_secret_key_len = 0;

  int pub_key_package_index_ = 0;
  SX12XX_Radio_Number_t transmitting_radio_;
  ECDH rxEcdh;
};
