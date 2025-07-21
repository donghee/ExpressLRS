#pragma once

#include <Arduino.h>
#include <stubborn_sender.h>

#include "FHSS.h"
#include "SX1280Driver.h"
#include "common.h"
#include "uECDH.h"

#define HANDSHAKE_DATA_SIZE 34

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
    txEcdh.get_public_key(pub_key_, &pub_key_len_);
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
            DBGLN("TX: Timeout waiting for hello, restarting handshake");
            tx_handshake_state_ = INIT;
        }
        break;
      case RECV_HELLO:
        tx_handshake_state_ = SEND_HELLO;
        break;
      case SEND_HELLO:
        DBGLN("send hello");
        handshake_hello();
        while (Busy()) {}
        tx_handshake_state_ = SEND_ECDH_PUB_KEY;
        break;
      case SEND_ECDH_PUB_KEY:
        DBGLN("send ecdh pub key 0");
        handshake_send_ecdh_pub_key0();
        while (Busy()) {}
        DBGLN("send ecdh pub key 1");
        handshake_send_ecdh_pub_key1();
        while (Busy()) {}

        tx_handshake_state_ = WAIT_ECDH_PUB_KEY;
        timeout = millis() + 100;  // wait for 0.1 seconds
        Radio.RXnb();
        break;
      case WAIT_ECDH_PUB_KEY:
        if (millis() > timeout) {
          DBGLN("TX: Timeout waiting for ECDH public key, restarting handshake");
          tx_handshake_state_ = SEND_ECDH_PUB_KEY;
        }

       break;
      case RECV_ECDH_PUB_KEY:
        DBGLN("recv ecdh pub key");
        delay(1);

        tx_handshake_state_ = SEND_BYE;
        break;
      case SEND_BYE:
        DBGLN("send bye");
        // send five bye messages to ensure the receiver gets it
        handshake_bye(); while (Busy()) {}
        handshake_bye(); while (Busy()) {}
        handshake_bye(); while (Busy()) {}
        handshake_bye(); while (Busy()) {}
        handshake_bye(); while (Busy()) {}
        tx_handshake_state_ = DONE;
        break;
      case DONE:
        digitalWrite(GPIO_PIN_LED, !digitalRead(GPIO_PIN_LED));
        // tx_handshake_state_ = INIT;
        break;
    }
    if (millis() > handle_timeout) {
      DBGLN("RX: Timeout waiting for handshake, restarting");
      tx_handshake_state_ = INIT;
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
  };

  void HandleWaitHello() {
    if (memcmp(Radio.RXdataBuffer, "hello", 5) == 0) {
      DBGLN("got hello");
      tx_handshake_state_ = RECV_HELLO;
      return;
    }

    Radio.RXnb();
  };

  void HandleWaitEcdhPubKey() {
    if (Radio.RXdataBuffer[0] == 0xEC) {
      if (Radio.RXdataBuffer[1] == 0) {
        DBGLN("got ecdh pub key 0");
        memcpy(rx_public_key_, Radio.RXdataBuffer + 2, 32);
        tx_handshake_state_ = WAIT_ECDH_PUB_KEY;
        Radio.RXnb();
      } else if (Radio.RXdataBuffer[1] == 1) {
        DBGLN("got ecdh pub key 1");
        memcpy(rx_public_key_ + 32, Radio.RXdataBuffer + 2, 32);
        rx_public_key_len_ = 64;
        tx_handshake_state_ = RECV_ECDH_PUB_KEY;
      }
    }
    // Radio.RXnb();
  };

  int LeaKey(uint8_t *K, size_t &K_len, uint8_t *A, size_t &A_len, uint8_t *N,
             size_t &N_len) {
    if (State() != DONE) return -1;

    txEcdh.generate_secret_key((const char *)rx_public_key_, rx_public_key_len_);
    txEcdh.export_secret_key(tx_secret_key, &tx_secret_key_len);

    memcpy(K_, tx_secret_key, 16);
    memcpy(A_, tx_secret_key + 16, 16);
    memcpy(N_, tx_secret_key + 8, 16);

    memcpy(K, K_, 16);
    memcpy(A, A_, 16);
    memcpy(N, N_, 16);
    K_len = 16;
    A_len = 16;
    N_len = 16;

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

  void handshake_send_ecdh_pub_key0() {
    Busy(true);
    tx_buffer[0] = 0xEC; // prefix for ECDH public key
    tx_buffer[1] = 0;
    memcpy(tx_buffer+2, pub_key_, 32);
    Radio.TXnb(tx_buffer, 32+2, transmitting_radio_);
  };

  void handshake_send_ecdh_pub_key1() {
    Busy(true);
    tx_buffer[0] = 0xEC; // prefix for ECDH public key
    tx_buffer[1] = 1;
    memcpy(tx_buffer+2, pub_key_+32, 32);
    Radio.TXnb(tx_buffer, 32+2, transmitting_radio_);
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
  unsigned char rx_public_key_[64] = {0};
  size_t rx_public_key_len_ = 0;

  uint8_t tx_secret_key[32];
  size_t tx_secret_key_len = 0;

  ECDH txEcdh;

  SX12XX_Radio_Number_t transmitting_radio_;
  // tx buffer for radio transmission
  uint8_t tx_buffer[HANDSHAKE_DATA_SIZE] = {0};
};
