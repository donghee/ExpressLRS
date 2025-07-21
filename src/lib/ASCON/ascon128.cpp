#include "ascon128.h"

#include <fstream>

#include <Arduino.h>
extern HardwareSerial DebugSerial;

#define MAX_P_C_BYTE_LENGTH 256

Ascon128::Ascon128() {
  ASCON128x_reset(&ascon_TX);
  ASCON128x_reset(&ascon_RX);
}

Ascon128::~Ascon128() {
  // Clear sensitive data
  // memset(&ascon, 0, sizeof(ASCON_st));
}
void Ascon128::increment_nonce_counter(uint8_t *nonce)
{
	int i;
    for (i = 15; i >= 0; --i)
    {
        if (++nonce[i] != 0)
        {
            break;
        }
    }
}

// by Joungil Yun (2025.02.05.)
void Ascon128::increase_nonce_counter_up_to_32bits_increment(uint8_t *nonce, uint32_t increment)
{
	int i;
	uint32_t carry = increment;
	uint32_t temp;

    for (i = 15; i >= 0; --i)
    {
    	temp = nonce[i] + carry;
    	nonce[i] = (uint8_t)temp;
    	carry = temp >> 8;
        if (carry == 0)
        {
            break;
        }
    }
}



int Ascon128::init(const uint8_t* K_, uint32_t K_len_, 
                   const uint8_t* A_, uint32_t A_len_, 
                   uint8_t *N_, size_t N_len_) {
  int result;

  COUNTER_TX = 0; // 초기화
	COUNTER_RX = 0; // 초기화 (COUNTER_TX와 동일한 값으로)
	
  memcpy(K, K_, K_len_);
  memcpy(A, A_, A_len_);

  result = init();
  if (result < 0) {
    return -1;
  }

  return 0;
}

int Ascon128::init() {
  int result;

  // result = ASCON128x_set_init_params(&ascon_TX, K, 128, A, 16, 16);
  // K 16 bytes = 128 bits, A 16 bytes, T 2 bytes
  result = ASCON128x_set_init_params(&ascon_TX, K, 128, A, 16, 2);

  if (result < 0) {
    return -1;
  }

  // result = ASCON128x_set_init_params(&ascon_RX, K, 128, A, 16, 16);
  result = ASCON128x_set_init_params(&ascon_RX, K, 128, A, 16, 2);
  if (result < 0) {
    return -1;
  }

  return 0;
}

int Ascon128::encrypt(const uint8_t *plaintext, int plaintext_len, uint8_t *ciphertext) { // plaintext to data
  int result;

  result = ASCON128x_set_enc_params(&ascon_TX, (uint8_t *)plaintext, plaintext_len, N, 16); 
  if (result < 0) {
    return -1;
  }

  result = ASCON128x_enc(&ascon_TX);
  if (result < 0) {
    return -1;
  }

  // increment_nonce_counter(N);

  // memcpy(ciphertext, ascon_TX.CC, ascon_TX.CC_byte_length);
  ciphertext[0] = (uint8_t)(COUNTER_TX >> 8); // 2 bytes
  ciphertext[1] = (uint8_t)COUNTER_TX;
  memcpy((uint8_t *)ciphertext + 2, ascon_TX.T, 2);
  memcpy((uint8_t *)ciphertext + 4, ascon_TX.CC, plaintext_len);

  // counter up
  COUNTER_TX = (COUNTER_TX + 1) % 4096;

  return 2 + 2 + ascon_TX.CC_byte_length; // 2 bytes for counter + 2 bytes for T + ciphertext
}

int Ascon128::decrypt(const uint8_t *ciphertext, uint8_t ciphertext_len, uint8_t *plaintext) { // ciphertext -> plaintext
  int result;
  uint32_t plaintext_len = ciphertext_len - (2 + 2);

    // 4 bits Packet Counter added by RF transmit module
    COUNTER_4b_new = ciphertext[0] >> 4; // 4 bits
    COUNTER_4b_gap = (COUNTER_4b_new - COUNTER_4b + 16) % 16;

    // 12 bits Packet Counter added by RC security module
    COUNTER_RX_new = ((ciphertext[0] & 0x0F) << 8) | ciphertext[1]; // 12 bits
    COUNTER_RX_gap = (COUNTER_RX_new - COUNTER_RX + 4096) % 4096;

    if((COUNTER_RX_gap < 3000 && initStatus == 0) || (COUNTER_RX_gap < 500 && initStatus != 0))
    {
      // for(int i = 0; i < COUNTER_RX_gap; i++)
      // {
      //   increment_nonce_counter(N);
      // }
      // by Joungil Yun (2025.02.05.)
      // increase_nonce_counter_up_to_32bits_increment(N, COUNTER_RX_gap);

      COUNTER_RX = COUNTER_RX_new;
      COUNTER_4b = COUNTER_4b_new;

      initStatus = 1;
    }
 
  //result = ASCON128x_set_dec_params(&ascon_RX, ciphertext, ciphertext_len, N, 16, T);
  result =  ASCON128x_set_dec_params(&ascon_RX, ciphertext + 4, plaintext_len, N, 16, ciphertext + 2);
  if (result < 0) {
    return -1;
  }

  result = ASCON128x_dec(&ascon_RX);
  if (result < 0) {
    return -1;
  }

  memcpy((uint8_t *)plaintext, (uint8_t *)ascon_RX.PP, plaintext_len);

  return plaintext_len;
}
