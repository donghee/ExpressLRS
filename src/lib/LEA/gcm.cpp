#include "gcm.h"
#include "OTA.h"

#include <fstream>

extern HardwareSerial DebugSerial;

GCM::GCM()
{
}

void GCM::increment_nonce_counter(uint8_t *nonce)
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

void GCM::increase_nonce_counter_up_to_32bits_increment(uint8_t *nonce, uint32_t increment)
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

int GCM::init(const uint8_t* K_, uint32_t K_len_, const uint8_t* A_, uint32_t A_len_, uint8_t *N_, size_t N_len_)
{
    int result;

    memcpy(K, K_, K_len_);
    memcpy(A, A_, A_len_);
    memcpy(N, N_, N_len_);

    result = init();
    if (result < 0) {
        return -1;
    }

    return 0;
}

int GCM::init()
{
    int result;

    // 카운터 초기화, initStatus 초기화
	COUNTER_TX = 0;
	COUNTER_RX = 0;
	initStatus = 0;

    // Kbits= 128, Abytes=16, Tbits = 16
    result = GCM4LEA_set_init_params(&gcm_TX, K, 128, A, 16, 16); // Last argument Tbits is 16

    if (result < 0) {
        return -1;
    }

    // Tbits = 16 for nonce sync, so gcm_RX.T is 2 bytes
    result = GCM4LEA_set_init_params(&gcm_RX, K, 128, A, 16, 16); // Last argument Tbits is 16
    if (result < 0) {
        return -1;
    }

    return 0;
}

int GCM::decrypt(const uint8_t *ciphertext, uint8_t ciphertext_len, uint8_t *plaintext) // ciphertext ->  plaintext
{
    int result;
    uint32_t plaintext_len = ciphertext_len - (2 + 2);

    // counter up
	COUNTER_RX_new = (ciphertext[0] << 8) | ciphertext[1]; // 2 bytes
	COUNTER_RX_gap = (COUNTER_RX_new - COUNTER_RX + 65536) % 65536;

    if((COUNTER_RX_gap < 3000 && initStatus == 0) || (COUNTER_RX_gap < 500 && initStatus != 0)) {
        increase_nonce_counter_up_to_32bits_increment(N, COUNTER_RX_gap);
		COUNTER_RX = COUNTER_RX_new;
        initStatus = 1;
	}
	else {
		// 초기화, 비정상적인 상황에 대한 예외처리
	}

    // DebugSerial.print("Nonce: ");
    // for (int j = 0; j < 16; j++) {
    //     DebugSerial.print(N[j], HEX);
    //     DebugSerial.print(" ");
    // }
    // DebugSerial.println(" ");

    // Tbits = 16 for nonce sync, so ciphertext + 2 is pointer of gcm_RX.T
   	result =  GCM4LEA_set_dec_params(&gcm_RX, ciphertext + 4, plaintext_len, N_GCM, 12, ciphertext + 2);
    if (result < 0) {
        return -1;
    }

    result = GCM4LEA_dec(&gcm_RX);
    if (result < 0) {
        return -1;
    }

    memcpy((uint8_t *)plaintext, (uint8_t *)gcm_RX.PP, plaintext_len);

    return plaintext_len;
}

int GCM::encrypt(const uint8_t *plaintext, int plaintext_len, uint8_t *ciphertext) // plaintext to data
{
    int result;

    result = GCM4LEA_set_enc_params(&gcm_TX, (uint8_t *)plaintext, plaintext_len, N_GCM, 12);
    if (result < 0) {
        return -1;
    }

    result =  GCM4LEA_enc(&gcm_TX);
    if (result < 0) {
        return -1;
    }

    ciphertext[0] = (uint8_t)(COUNTER_TX >> 8); // 2 bytes
    ciphertext[1] = (uint8_t)COUNTER_TX;
    memcpy((uint8_t *)ciphertext + 2, gcm_TX.T, 2);
    memcpy((uint8_t *)ciphertext + 4, gcm_TX.CC, plaintext_len);

    // counter up and nonce advance
    COUNTER_TX = (COUNTER_TX + 1) % 65536;
    increment_nonce_counter(N);

    // DebugSerial.print("Nonce: ");
    // for (int j = 0; j < 16; j++) {
    //     DebugSerial.print(N[j], HEX);
    //     DebugSerial.print(" ");
    // }
    // DebugSerial.println(" ");

    return 2 + 2 + gcm_TX.CC_byte_length; // 2 bytes for counter + 2 bytes for T + ciphertext
}

// RX
int GCM::decrypt(OTA_Packet_s *otaPktPtr, const uint8_t *data, uint8_t dataLen) // data --> otaPktPtr
{
    int ret = 0;
    ret = this->decrypt((uint8_t *)data, dataLen, (uint8_t *)otaPktPtr);
    return ret;
}

// TX
int GCM::encrypt(OTA_Packet_s *otaPktPtr, uint8_t *data, uint8_t dataLen) // otaPktPtr -> data
{
    int ret = 0;
    ret = this->encrypt((uint8_t *)otaPktPtr, sizeof(OTA_Packet_s), (uint8_t *)data);
    return ret;
}
