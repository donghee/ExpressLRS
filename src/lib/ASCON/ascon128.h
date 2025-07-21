#pragma once

#include <cstdint>

extern "C"
{
#include "ascon128x.h"
#include <stddef.h>
}

class Ascon128 {
private:
  ASCON_st ascon_TX;
  ASCON_st ascon_RX;

  // ASCON
  uint8_t K[16] = {0x14, 0x87, 0x0B, 0x99, 0x92, 0xEA, 0x89, 0x67, 0x8A, 0x1D, 0xDF, 0xD6, 0x30, 0x91, 0x8D, 0xF0};
  uint8_t A[16] = {0, };
  uint8_t N[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F }; // random 값 사용을 권장함!!!!!

  uint8_t T[16] = {0, };

    uint16_t COUNTER_TX;
    uint16_t COUNTER_RX;
    uint16_t COUNTER_RX_new;
    uint16_t COUNTER_RX_gap;
    int initStatus = 0;

    uint8_t COUNTER_4b;
    uint8_t COUNTER_4b_new;
    uint8_t COUNTER_4b_gap;


public:
  Ascon128();
  ~Ascon128();

  int init();

  int init(const uint8_t* K_, uint32_t K_len_, 
                   const uint8_t* A_, uint32_t A_len_, 
                   uint8_t *N_, size_t N_len_);

  void increment_nonce_counter(uint8_t *nonce);
  void increase_nonce_counter_up_to_32bits_increment(uint8_t *nonce, uint32_t increment);

  int encrypt(const uint8_t *plaintext, int plaintext_len, uint8_t *ciphertext); // plaintext to data
  int decrypt(const uint8_t *ciphertext, uint8_t ciphertext_len, uint8_t *plaintext); // ciphertext ->  plaintext
};

