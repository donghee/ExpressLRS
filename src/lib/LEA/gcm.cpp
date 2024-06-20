#include "gcm.h"
#include "OTA.h"

#include <fstream>

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

int GCM::init(uint8_t *K_, size_t K_len_, uint8_t *A_, size_t A_len_, uint8_t *N_, size_t N_len_)
{
    int is_ok = -1;

    memcpy(K, K_, K_len_);
    memcpy(A, A_, A_len_);
    memcpy(N, N_, N_len_);

    // 1. 바이딩 초기화 단계 키 교환 과정에서 Nonce 초기값도 암호화된 방식으로 함께 공유 (연결이 끊겨 리셋 될 때마다 수행되어야 함)
	COUNTER_TX = 0; // 초기화
	COUNTER_RX = 0; // 초기화 (COUNTER_TX와 동일한 값으로)
	initStatus = 1;

    is_ok = init();

    return is_ok;
}

int GCM::init()
{
    int is_ok = -1;

    // Kbits= 128, Abytes=16, Tbits = 32
    // TODO: To reduce the packet size, the bit size of T must be reduced. 8 * 16 = 128
    // if (GCM4LEA_set_init_params(&gcm_TX, K, 128, A, 16, 32))
    // Tbits = 16 for nonce sync, so gcm_TX.T is 2 bytes
    if (GCM4LEA_set_init_params(&gcm_TX, K, 128, A, 16, 16))
    {
        return is_ok; // Error
    }

    // TODO: delete other gcm_TX
    // if (GCM4LEA_set_init_params(&gcm_RX, K, 128, A, 16, 32))
    // Tbits = 16 for nonce sync, so gcm_RX.T is 2 bytes
    if (GCM4LEA_set_init_params(&gcm_RX, K, 128, A, 16, 16))
    {
        return is_ok; // Error
    }

    is_ok = 0;

    return is_ok;
}

// TX
int GCM::encrypt(OTA_Packet_s *otaPktPtr, uint8_t *data, uint8_t dataLen) // ota to data
{
    int is_ok = -1;

    if (GCM4LEA_set_enc_params(&gcm_TX, (uint8_t *)otaPktPtr, 16, N, 12))
    {
        return is_ok; // Error
    }
    if (GCM4LEA_enc(&gcm_TX))
    {
        return is_ok; // Error
    }

    // TODO: To reduce the packet size, the bit size of T must be reduced.
    // memcpy((uint8_t *)data, gcm_TX.T, 4);

    // counter up
    COUNTER_TX = (COUNTER_TX + 1) % 65536;
    increment_nonce_counter(N);

    data[0] = (uint8_t)(COUNTER_TX >> 8); // 2 bytes
    data[1] = (uint8_t)COUNTER_TX;
    memcpy((uint8_t *)data + 2, gcm_TX.T, 2);
    memcpy((uint8_t *)data + 4, gcm_TX.CC, 16);

    is_ok = 0;

    return is_ok;
}

// RX
int GCM::decrypt(OTA_Packet_s *otaPktPtr, const uint8_t *data, uint8_t dataLen) // data --> otaPktPtr
{
    int is_ok = -1;

    // counter up
	COUNTER_RX_new = (data[0] << 8) | data[1]; // 2 bytes
	COUNTER_RX_gap = (COUNTER_RX_new - COUNTER_RX + 65536) % 65536;

  	// [Note] 만일 COUNTER_RX_gap = 0이면 초기화 직후 송수신으로 판별할 수 있음 (0이 반복되는 경우에도 정상적인 상황이 아니기 때문에 이에 대한 처리도 필요함!!!!!!!!!)
	// [Note] 만인 COUNTER_RX_gap = 1이면 초기화 단계 후에 전상적으로 1씩 증가된 송수신으로 판별할 수 있음
	// [Note] 만일 COUNTER_Rx_gap이 큰 값이면 신호가 끊긴 시간이 길거나 정상적이지 않은 상황으로 판별하고 초기화를 재시도
	if (COUNTER_RX_gap < 200) // 200은 적당히 작은 값으로 변경 (실험 후)
	{
		for(int i = 0; i < COUNTER_RX_gap; i++)
		{
	    	increment_nonce_counter(N);
		}

		COUNTER_RX = COUNTER_RX_new;

        // if (COUNTER_RX_new > 500) // 50hz * 10s = 500
        //     __BKPT();
	}
	else
	{
        // TODO
		// 초기화, 비정상적인 상황에 대한 예외처리
	}

    // if (GCM4LEA_set_dec_params(&gcm_RX, data + 4, 16, N, 12, data))
    // Tbits = 16 for nonce sync, so data + 2 is pointer of gcm_RX.T
   	if (GCM4LEA_set_dec_params(&gcm_RX, data + 4, 16, N, 12, data + 2))
    {
        return is_ok; // Error
    }
    if (GCM4LEA_dec(&gcm_RX))
    {
        return is_ok; // Error
    }

    // otaPktPtr = (OTA_Packet_s *)gcm_RX.PP;
    memcpy((uint8_t *)otaPktPtr, (uint8_t *)gcm_RX.PP, 16);

    is_ok = 0;

    return is_ok;
}
