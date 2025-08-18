#pragma once

#include <Arduino.h>
#include <stubborn_sender.h>

#include "FHSS.h"
#include "SX1280Driver.h"
#include "common.h"
#include "uECDH.h"

/** @brief Maximum size for handshake data transmission buffer */
#define HANDSHAKE_DATA_SIZE 34

/**
 * @brief Receiver-side ECDH handshake protocol implementation
 *
 * This class implements the receiver side of the ECDH-based secure handshake
 * protocol for ExpressLRS radio communication. The handshake establishes a
 * shared encryption key between transmitter and receiver using elliptic curve
 * cryptography, ensuring secure communication channels.
 *
 * Protocol Flow (RX side):
 * 1. Send initial "hello" message to announce presence
 * 2. Wait for and receive TX "hello" response
 * 3. Wait for TX ECDH public key (sent in two 32-byte parts)
 * 4. Send own ECDH public key to TX (in two 32-byte parts)
 * 5. Wait for "bye" confirmation from TX
 * 6. Generate shared secret key for encryption
 *
 * Features:
 * - State machine-based handshake protocol
 * - Timeout handling and automatic retry logic
 * - 512-bit ECDH key exchange
 * - Fragmented public key transmission (64 bytes split into 2x32 bytes)
 * - Shared secret derivation for LEA/ASCON encryption keys
 *
 * Written by: Donghee Park (DRONEMAP)
 */
class RxHandshakeClass {
 public:
  /**
   * @brief Handshake protocol state enumeration
   *
   * Defines the various states of the RX handshake state machine,
   * from initialization through completion.
   */
  enum handshake_state_t {
    INIT = 0,              /**< Initial state - prepare for handshake */
    SEND_HELLO,            /**< Send hello message to TX */
    WAIT_HELLO,            /**< Wait for hello response from TX */
    RECV_HELLO,            /**< Process received hello from TX */
    WAIT_ECDH_PUB_KEY,     /**< Wait for TX ECDH public key */
    RECV_ECDH_PUB_KEY,     /**< Process received TX public key */
    SEND_ECDH_PUB_KEY,     /**< Send own ECDH public key to TX */
    WAIT_BYE,              /**< Wait for bye confirmation from TX */
    DONE                   /**< Handshake completed successfully */
  };

  /**
   * @brief Default constructor for RX handshake manager
   */
  RxHandshakeClass() { }

  /**
   * @brief Initialize the RX handshake protocol
   *
   * Sets up the handshake state machine, initializes ECDH key pair,
   * and prepares for secure communication establishment.
   */
  void Init() {
    rx_handshake_state_ = INIT;
    busy_transmitting_ = false;
    transmitting_radio_ = Radio.GetLastSuccessfulPacketRadio();

    rxEcdh.init();
    rxEcdh.get_public_key(pub_key_, &pub_key_len_);
  }

  /**
   * @brief Check if handshake protocol has completed successfully
   * @return true if handshake is complete, false otherwise
   */
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
        delay(1);

        rx_handshake_state_ = SEND_ECDH_PUB_KEY;
        break;
      case SEND_ECDH_PUB_KEY:
        DBGLN("send ecdh pub key 0");
        handshake_send_ecdh_pub_key0();
        while (Busy()) {}
        DBGLN("send ecdh pub key 0");
        handshake_send_ecdh_pub_key1();
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
        // rx_handshake_state_ = INIT;
        break;
    }
    if (millis() > handle_timeout) {
      DBGLN("RX: Timeout waiting for handshake, restarting");
      rx_handshake_state_ = INIT;
    }
  };

  /**
   * @brief Callback function called when radio transmission is complete
   *
   * Clears the busy transmission flag to allow next transmission.
   */
  void TXdoneCallback() { Busy(false); };

  /**
   * @brief Callback function called when radio reception is complete
   * @param[in] status Reception status from radio driver
   * @return true if packet was handled by handshake protocol, false otherwise
   *
   * Routes received packets to appropriate handlers based on current
   * handshake state (hello, ECDH key exchange, bye confirmation).
   */
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

  /**
   * @brief Generate Cryto encryption keys from ECDH shared secret
   * @param[out] K Output buffer for LEA encryption key (16 bytes)
   * @param[out] K_len Length of encryption key (set to 16)
   * @param[out] A Output buffer for associated data key (16 bytes)
   * @param[out] A_len Length of associated data (set to 16)
   * @param[out] N Output buffer for nonce/IV (16 bytes)
   * @param[out] N_len Length of nonce (set to 16)
   * @return 0 on success, -1 if handshake not completed
   *
   * Derives LEA encryption parameters from the ECDH shared secret:
   * - K: bytes 0-15 of shared secret (encryption key)
   * - A: bytes 16-31 of shared secret (associated data)
   * - N: bytes 8-23 of shared secret (nonce/IV)
   */
  int LeaKey(uint8_t *K, size_t &K_len, uint8_t *A, size_t &A_len, uint8_t *N,
             size_t &N_len) {
    if (State() != DONE) return -1;

    rxEcdh.generate_secret_key((const char *)tx_public_key_, tx_public_key_len_);
    rxEcdh.export_secret_key(rx_secret_key, &rx_secret_key_len);

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

  void HandleWaitHello() {
    if (memcmp(Radio.RXdataBuffer, "hello", 5) == 0) {
      DBGLN("got hello");
      rx_handshake_state_ = RECV_HELLO;
      return;
    }

    Radio.RXnb();
  };

  void HandleWaitEcdhPubKey() {
    if (Radio.RXdataBuffer[0] == 0xEC) {
      if (Radio.RXdataBuffer[1] == 0) {
        DBGLN("got ecdh pub key0");
        memcpy(tx_public_key_, Radio.RXdataBuffer + 2, 32);
        rx_handshake_state_ = WAIT_ECDH_PUB_KEY;
        Radio.RXnb();
      } else if (Radio.RXdataBuffer[1] == 1) {
        DBGLN("got ecdh pub key1");
        memcpy(tx_public_key_ + 32, Radio.RXdataBuffer + 2, 32);
        tx_public_key_len_ = 64;
        rx_handshake_state_ = RECV_ECDH_PUB_KEY;
      }
    }
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
  unsigned char tx_public_key_[64] = {0};
  size_t tx_public_key_len_ = 0;

  uint8_t rx_secret_key[32];
  size_t rx_secret_key_len = 0;

  ECDH rxEcdh;

  SX12XX_Radio_Number_t transmitting_radio_;
  // tx buffer for radio transmission
  uint8_t tx_buffer[HANDSHAKE_DATA_SIZE] = {0};
};
