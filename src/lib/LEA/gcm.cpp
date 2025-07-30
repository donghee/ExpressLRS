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

uint32_t GCM::encryption_time()
{
    delta[1] = stop[1] - start[1];
    return delta[1];
}

uint32_t GCM::decryption_time()
{
    delta[2] = stop[2] - start[2];
    return delta[2];
}

// int GCM::init(uint8_t *K_, size_t K_len_, uint8_t *A_, size_t A_len_, uint8_t *N_, size_t N_len_)
int GCM::init(const uint8_t* K_, uint32_t K_len_, const uint8_t* A_, uint32_t A_len_, uint8_t *N_, size_t N_len_)
{
    int result;

    // Measurement of lea encryption and decryption time
    if (ARM_CM_DWT_CTRL != 0) 			// See if DWT is available
    {
		ARM_CM_DEMCR      |= 1 << 24;	// Set bit 24
		ARM_CM_DWT_CYCCNT  = 0;
		ARM_CM_DWT_CTRL   |= 1 << 0;	// Set bit 0
    }

    memcpy(K, K_, K_len_);
    memcpy(A, A_, A_len_);
    memcpy(N, N_, N_len_);

    // 1. 바이딩 초기화 단계 키 교환 과정에서 Nonce 초기값도 암호화된 방식으로 함께 공유 (연결이 끊겨 리셋 될 때마다 수행되어야 함)
	COUNTER_TX = 0; // 초기화
	COUNTER_RX = 0; // 초기화 (COUNTER_TX와 동일한 값으로)
	initStatus = 1;

    result = init();
    if (result < 0) {
        return -1;
    }

    return 0;
}

int GCM::init()
{
    int result;

    // Kbits= 128, Abytes=16, Tbits = 32
    // TODO: To reduce the packet size, the bit size of T must be reduced. 8 * 16 = 128
    // if (GCM4LEA_set_init_params(&gcm_TX, K, 128, A, 16, 32))
    // Tbits = 16 for nonce sync, so gcm_TX.T is 2 bytes
    start[0] = ARM_CM_DWT_CYCCNT;
    result = GCM4LEA_set_init_params(&gcm_TX, K, 128, A, 16, 16); // Last argument is Tbits = 16
    stop[0] = ARM_CM_DWT_CYCCNT;

    if (result < 0) {
        return -1;
    }

    // TODO: delete other gcm_TX
    // if (GCM4LEA_set_init_params(&gcm_RX, K, 128, A, 16, 32))
    // Tbits = 16 for nonce sync, so gcm_RX.T is 2 bytes
    result = GCM4LEA_set_init_params(&gcm_RX, K, 128, A, 16, 16);
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

    if((COUNTER_RX_gap < 3000 && initStatus == 0) || (COUNTER_RX_gap < 500 && initStatus != 0))
	{
		// for(int i = 0; i < COUNTER_RX_gap; i++)
		// {
		//     increment_nonce_counter(N);
		// }
        increase_nonce_counter_up_to_32bits_increment(N, COUNTER_RX_gap);

		COUNTER_RX = COUNTER_RX_new;

        initStatus = 1;
	}
	else
	{
        // TODO
		// 초기화, 비정상적인 상황에 대한 예외처리
	}

    // Tbits = 16 for nonce sync, so ciphertext + 2 is pointer of gcm_RX.T
   	result =  GCM4LEA_set_dec_params(&gcm_RX, ciphertext + 4, plaintext_len, N, 12, ciphertext + 2);
    if (result < 0) {
        return -1;
    }

    start[2] = ARM_CM_DWT_CYCCNT;
    result = GCM4LEA_dec(&gcm_RX);
    stop[2] = ARM_CM_DWT_CYCCNT;
    if (result < 0) {
        return -1;
    }

    memcpy((uint8_t *)plaintext, (uint8_t *)gcm_RX.PP, plaintext_len);

    return plaintext_len;
}

int GCM::encrypt(const uint8_t *plaintext, int plaintext_len, uint8_t *ciphertext) // plaintext to data
{
    int result;

    result = GCM4LEA_set_enc_params(&gcm_TX, (uint8_t *)plaintext, plaintext_len, N, 12);
    if (result < 0) {
        return -1;
    }

    start[1] = ARM_CM_DWT_CYCCNT;
    result =  GCM4LEA_enc(&gcm_TX);
    stop[1] = ARM_CM_DWT_CYCCNT;
    if (result < 0) {
        return -1;
    }

    // counter up
    COUNTER_TX = (COUNTER_TX + 1) % 65536;
    increment_nonce_counter(N);

    ciphertext[0] = (uint8_t)(COUNTER_TX >> 8); // 2 bytes
    ciphertext[1] = (uint8_t)COUNTER_TX;
    memcpy((uint8_t *)ciphertext + 2, gcm_TX.T, 2);
    memcpy((uint8_t *)ciphertext + 4, gcm_TX.CC, plaintext_len);

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
