#pragma once

#include <Arduino.h>
#include "SX1280Driver.h"
#include "common.h"
#include "FHSS.h"
#include "rsa.h"

enum rx_handshake_state_t {
    HANDSHAKE_INIT,
    HANDSHAKE_WAIT_HELLO,
    HANDSHAKE_WAIT_RSA_PUB_KEY,
    HANDSHAKE_GOT_RSA_PUB_KEY,
    HANDSHAKE_SEND_ACK,
    HANDSHAKE_SEND_LEA_KEY,
    HANDSHAKE_DONE
};

class RxHandshakeClass {
 public:

  RxHandshakeClass() {
    init();
  }

  void init() {
    rx_handshake_state = HANDSHAKE_INIT;
    busyTransmitting = false;
    transmittingRadio = Radio.GetLastSuccessfulPacketRadio();
  }

  rx_handshake_state_t state() { return rx_handshake_state; };

  bool busy() { return busyTransmitting; };
  void busy(bool busyTransmitting_) { busyTransmitting = busyTransmitting_; };

  void do_handle() {
    switch (rx_handshake_state) {
      case HANDSHAKE_INIT:
        delay(500);
        rx_handshake_state = HANDSHAKE_WAIT_HELLO;
        Radio.RXnb();
        break;

      case HANDSHAKE_WAIT_HELLO:
        break;

      case HANDSHAKE_GOT_RSA_PUB_KEY:
        // encrypt LEA key using RSA public key
        memcpy(plaintext, K, 16);
        memcpy(plaintext + 16, A, 16);
        memcpy(plaintext + 32, N, 12);
        encrypt(pub_key, pub_key_len, plaintext, 16+16+12, ciphertext_pub, 512);
        //TODO: if encrypt success, send ack otherwise send nack
        rx_handshake_state = HANDSHAKE_SEND_ACK;
        break;

      case HANDSHAKE_SEND_ACK:
        if (!busyTransmitting) {
          busyTransmitting = true;
          Radio.TXnb((uint8_t*)"ack", 3, transmittingRadio);
        }
        while (busyTransmitting) { }
        delayMicroseconds(100);
        ack++;
        if (ack == 20) { // ack 20번 보내면, TX에서 LEA키 받고, 5번 보내면, TX에 LEA키 이상한 값이 나옴
          ack = 0;
          rx_handshake_state = HANDSHAKE_SEND_LEA_KEY;
        }
        break;

      case HANDSHAKE_SEND_LEA_KEY:
        for (int i = 0; i < 128; i=i+8) {
          busyTransmitting = true;
          Radio.TXnb(ciphertext_pub+i, 8, transmittingRadio);
          while (busyTransmitting) { }
        }
        rx_handshake_state = HANDSHAKE_DONE;
        break;

      case HANDSHAKE_DONE:
        digitalWrite(GPIO_PIN_LED, !digitalRead(GPIO_PIN_LED));
        rx_handshake_state = HANDSHAKE_INIT;
        break;
    }
  };

  void handle_wait_hello() {
    if (memcmp(Radio.RXdataBuffer, "hello", 5) == 0) {
      rx_handshake_state = HANDSHAKE_WAIT_RSA_PUB_KEY;
      pub_key_packageIndex = 0;
      pub_key_len = 0;
    }

    Radio.RXnb();
  };

  void handle_recv_rsa_pub_key() {
    if (pub_key_packageIndex >= 32) { // 256 bytes
      rx_handshake_state = HANDSHAKE_GOT_RSA_PUB_KEY;
      return;
    }
    memcpy(pub_key + (pub_key_packageIndex * 8), Radio.RXdataBuffer, 8);
    pub_key_packageIndex++;
    pub_key_len += 8;

    Radio.RXnb();
  };

  void TXdoneCallback() {
    busyTransmitting = false;
  };


  void encrypt(unsigned char *pub_key, size_t pub_key_len,
                      unsigned char *plaintext, size_t plaintext_len,
                      unsigned char *ciphertext, size_t ciphertext_len)
  {
    int ret = 1;
    const char *pers = "rsa_genkey";

    RSA rsa;
    // generate key for test
    ret = rsa.generate_key(pers, strlen(pers));
    if ( ret != 0 ) {
       DBGLN("FAILED GENERATE KEY");
       __BKPT();
       return;
    }

    // encrypt using generated public key
    ret = rsa.encrypt(pub_key, pub_key_len, plaintext, plaintext_len, ciphertext);
    if ( ret != 0 ) {
        __BKPT();
        return;
    }
  };

 private:
  rx_handshake_state_t rx_handshake_state;
  bool busyTransmitting;

  int ack = 0;

  // LEA key
  unsigned char plaintext[1024] = {0};

  // generate plaintext for test
  uint8_t K[16] = {0x14, 0x87, 0x0B, 0x99, 0x92, 0xEA, 0x89, 0x67, 0x8A, 0x1D, 0xDF, 0xD6, 0x30, 0x91, 0x8D, 0xF0};
  uint8_t A[16] = {0};
  uint8_t N[12] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B};

  // encrypted ciphertext from plaintext (K,A,N)
  unsigned char ciphertext_pub[512] = {0};

  // rsa
  unsigned char pub_key[1024] = {0};
  size_t pub_key_len = 0;
  int pub_key_packageIndex = 0;
  size_t packageSize = 8;
  SX12XX_Radio_Number_t transmittingRadio;
};
