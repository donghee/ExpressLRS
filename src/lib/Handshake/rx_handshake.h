#pragma once

#include <Arduino.h>

#include "FHSS.h"
#include "SX1280Driver.h"
#include "common.h"
#include "rsa.h"

#define DATA_SIZE 32

class RxHandshakeClass {
 public:
  enum handshake_state_t {
    INIT = 0,
    WAIT_HELLO,
    WAIT_RSA_PUB_KEY,
    GOT_RSA_PUB_KEY,
    SEND_ACK,
    SEND_LEA_KEY,
    DONE
  };

  RxHandshakeClass() { Init(); }

  void Init() {
    rx_handshake_state_ = INIT;
    busy_transmitting_ = false;
    transmitting_radio_ = Radio.GetLastSuccessfulPacketRadio();
  }

  inline bool IsDone() { return rx_handshake_state_ == DONE; };

  void DoHandle() {
    int ret = 1;

    switch (rx_handshake_state_) {
      case INIT:
        delay(500);
        rx_handshake_state_ = WAIT_HELLO;
        Radio.RXnb();
        break;

      case WAIT_HELLO:
        break;

      case GOT_RSA_PUB_KEY:
        // encrypt LEA key using RSA public key
        memcpy(plaintext_, K_, 16);
        memcpy(plaintext_ + 16, A_, 16);
        memcpy(plaintext_ + 32, N_, 12);
        Encrypt(pub_key_, pub_key_len_, plaintext_, 16 + 16 + 12,
                ciphertext_pub_, 512);
        // TODO: if encrypt success, send ack otherwise send nack
        rx_handshake_state_ = SEND_ACK;
        break;

      case SEND_ACK:
        if (!Busy()) {
          Busy(true);
          Radio.TXnb((uint8_t *)"ack", 3, transmitting_radio_);
        }
        while (Busy()) {
        }
        delayMicroseconds(100);
        ack_++;
        if (ack_ == 20) {  // ack 20번 보내면, TX에서 LEA키 받고, 5번 보내면,
                           // TX에 LEA키 이상한 값이 나옴
          ack_ = 0;
          rx_handshake_state_ = SEND_LEA_KEY;
        }
        break;

      case SEND_LEA_KEY:
        for (int i = 0; i < 128; i = i + 8) {
          Busy(true);
          Radio.TXnb(ciphertext_pub_ + i, 8, transmitting_radio_);
          while (Busy()) {
          }
        }
        rx_handshake_state_ = DONE;
        break;

      case DONE:
        digitalWrite(GPIO_PIN_LED, !digitalRead(GPIO_PIN_LED));
        rx_handshake_state_ = INIT;
        break;
    }
  };

  void TXdoneCallback() { Busy(false); };

  bool RXdoneCallback(SX12xxDriverCommon::rx_status const status) {
    if (State() == WAIT_HELLO) {
      HandleWaitHello();
      return true;
    }

    if (State() == WAIT_RSA_PUB_KEY) {
      HandleRecvRsaPubKey();
      return true;
    }
  };

 private:
  inline void Busy(bool busyTransmitting_) {
    busy_transmitting_ = busyTransmitting_;
  };

  handshake_state_t State() { return rx_handshake_state_; };

  inline bool Busy() { return busy_transmitting_; };

  void Encrypt(unsigned char *pub_key, size_t pub_key_len,
               unsigned char *plaintext, size_t plaintext_len,
               unsigned char *ciphertext, size_t ciphertext_len) {
    int ret = 1;
    const char *pers = "rsa_genkey";

    RSA rsa;
    // generate key for test
    ret = rsa.generate_key(pers, strlen(pers));
    if (ret != 0) {
      DBGLN("FAILED GENERATE KEY");
      __BKPT();
      return;
    }

    // encrypt using generated public key
    ret =
        rsa.encrypt(pub_key, pub_key_len, plaintext, plaintext_len, ciphertext);
    if (ret != 0) {
      __BKPT();
      return;
    }
  };

  void HandleWaitHello() {
    if (memcmp(Radio.RXdataBuffer, "hello", 5) == 0) {
      rx_handshake_state_ = WAIT_RSA_PUB_KEY;
      pub_key_package_index_ = 0;
      pub_key_len_ = 0;
    }

    Radio.RXnb();
  };

  void HandleRecvRsaPubKey() {
    if (pub_key_package_index_ >= 32) {  // 256 bytes
      rx_handshake_state_ = GOT_RSA_PUB_KEY;
      return;
    }
    memcpy(pub_key_ + (pub_key_package_index_ * 8), Radio.RXdataBuffer, 8);
    pub_key_package_index_++;
    pub_key_len_ += 8;

    Radio.RXnb();
  };

  handshake_state_t rx_handshake_state_;
  bool busy_transmitting_;

  int ack_ = 0;

  // LEA key
  unsigned char plaintext_[1024] = {0};

  // generate plaintext for test
  uint8_t K_[16] = {0x14, 0x87, 0x0B, 0x99, 0x92, 0xEA, 0x89, 0x67,
                    0x8A, 0x1D, 0xDF, 0xD6, 0x30, 0x91, 0x8D, 0xF0};
  uint8_t A_[16] = {0};
  uint8_t N_[12] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
                    0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B};

  // encrypted ciphertext from plaintext (K,A,N)
  unsigned char ciphertext_pub_[512] = {0};

  // rsa
  unsigned char pub_key_[1024] = {0};
  size_t pub_key_len_ = 0;
  int pub_key_package_index_ = 0;
  SX12XX_Radio_Number_t transmitting_radio_;
};
