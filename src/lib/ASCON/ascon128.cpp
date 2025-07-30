#include "ascon128.h"

#include <fstream>

#include <Arduino.h>

// void __attribute__ ((noinline)) breakpoint()
// {
//     __asm("NOP");
// }
//
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

uint32_t Ascon128::encryption_time()
{
    delta[1] = stop[1] - start[1];
    return delta[1];
}

uint32_t Ascon128::decryption_time()
{
    delta[2] = stop[2] - start[2];
    return delta[2];
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

  // K 16 bytes = 128 bits, A 16 bytes, T 2 bytes
  start[0] = ARM_CM_DWT_CYCCNT;
  result = ASCON128x_set_init_params(&ascon_TX, K, 128, A, 16, 2); // TBytes 2 bytes
  stop[0] = ARM_CM_DWT_CYCCNT;

  if (result < 0) {
    return -1;
  }

  result = ASCON128x_set_init_params(&ascon_RX, K, 128, A, 16, 2); // Last argument is TBytes 2 bytes
  if (result < 0) {
    return -1;
  }

  return 0;
}

int Ascon128::encrypt(const uint8_t *plaintext, int plaintext_len, uint8_t *ciphertext) { // plaintext to data
    int result;

    start[1] = ARM_CM_DWT_CYCCNT;
    result = ASCON128x_set_enc_params(&ascon_TX, (uint8_t *)plaintext, plaintext_len, N, 16);
    stop[1] = ARM_CM_DWT_CYCCNT;
    if (result < 0) {
        return -1;
    }

    result = ASCON128x_enc(&ascon_TX);
    if (result < 0) {
        return -1;
    }

    // counter up
    COUNTER_TX = (COUNTER_TX + 1) % 65536;
    // increment_nonce_counter(N);

    ciphertext[0] = (uint8_t)(COUNTER_TX >> 8); // 2 bytes
    ciphertext[1] = (uint8_t)COUNTER_TX;
    memcpy((uint8_t *)ciphertext + 2, ascon_TX.T, 2);
    memcpy((uint8_t *)ciphertext + 4, ascon_TX.CC, plaintext_len);

    return 2 + 2 + ascon_TX.CC_byte_length; // 2 bytes for counter + 2 bytes for T + ciphertext
}

int Ascon128::decrypt(const uint8_t *ciphertext, uint8_t ciphertext_len, uint8_t *plaintext) { // ciphertext -> plaintext
    int result;
    uint32_t plaintext_len = ciphertext_len - (2 + 2);

    // counter up
    COUNTER_RX_new = (ciphertext[0] << 8) | ciphertext[1]; // 2 bytes
    COUNTER_RX_gap = (COUNTER_RX_new - COUNTER_RX + 65536) % 65536;

    if((COUNTER_RX_gap < 3000 && initStatus == 0) || (COUNTER_RX_gap < 500 && initStatus != 0))
    {
        // for(int i = 0; i < COUNTER_RX_gap; i++)
        // {
        //   increment_nonce_counter(N);
        // }
        // increase_nonce_counter_up_to_32bits_increment(N, COUNTER_RX_gap);

        COUNTER_RX = COUNTER_RX_new;

        initStatus = 1;
    }
    else
	{
        // TODO
		// 초기화, 비정상적인 상황에 대한 예외처리
	}

    // Tbits = 16 for nonce sync, so ciphertext + 2 is pointer of gcm_RX.T
    result =  ASCON128x_set_dec_params(&ascon_RX, ciphertext + 4, plaintext_len, N, 16, ciphertext + 2);
    if (result < 0) {
        return -1;
    }

    start[2] = ARM_CM_DWT_CYCCNT;
    result = ASCON128x_dec(&ascon_RX);
    stop[2] = ARM_CM_DWT_CYCCNT;
    if (result < 0) {
        return -1;
    }

    memcpy((uint8_t *)plaintext, (uint8_t *)ascon_RX.PP, plaintext_len);

    return plaintext_len;
}

int Ascon128::encrypt(OTA_Packet_s *otaPktPtr, uint8_t *data, uint8_t dataLen) {
    int ret = 0;

    // DebugSerial.print("plaintext: ");
    // for (int i = 0; i < sizeof(OTA_Packet_s); i++) {
    //     DebugSerial.print(((uint8_t *)otaPktPtr)[i], HEX);
    // }
    // DebugSerial.println();

    ret = this->encrypt((uint8_t *)otaPktPtr, sizeof(OTA_Packet_s), (uint8_t *)data);

    // DebugSerial.print("ciphertext: -> ");
    // for (int i = 0; i < dataLen; i++) {
    //     DebugSerial.print(((uint8_t *)data)[i], HEX);
    // }
    // DebugSerial.println();

    return ret;

}

int Ascon128::decrypt(OTA_Packet_s *otaPktPtr, const uint8_t *data, uint8_t dataLen) {
    int ret = 0;

    // DebugSerial.println();
    // DebugSerial.print("ciphertext: <- ");
    // for (int i = 0; i < dataLen; i++) {
    //     DebugSerial.print(((uint8_t *)data)[i], HEX);
    // }
    // DebugSerial.println();

    ret = this->decrypt((uint8_t *)data, dataLen, (uint8_t *)otaPktPtr);

    // DebugSerial.print("decrypted text: ");
    // for (int i = 0; i < sizeof(OTA_Packet_s); i++) {
    //     DebugSerial.print(((uint8_t *)otaPktPtr)[i], HEX);
    // }
    // DebugSerial.println();

    return ret;
}
