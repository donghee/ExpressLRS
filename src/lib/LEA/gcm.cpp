#include "gcm.h"
#include "OTA.h"

#include <fstream>

GCM::GCM()
{
}

int GCM::init(uint8_t *K_, size_t K_len_, uint8_t *A_, size_t A_len_, uint8_t *N_, size_t N_len_)
{
    int is_ok = -1;

    memcpy(K, K_, K_len_);
    memcpy(A, A_, A_len_);
    memcpy(N, N_, N_len_);

    // TODO: Refactor this to use the next init function
    // TODO: To reduce the packet size, the bit size of T must be reduced. 8 * 16 = 128
    if (GCM4LEA_set_init_params(&gcm_TX, K, 128, A, 16, 32))
    {
        return is_ok; // Error
    }
    if (GCM4LEA_set_init_params(&gcm_RX, K, 128, A, 16, 32))
    {
        return is_ok; // Error
    }

    is_ok = 0;

    return is_ok;
}

int GCM::init()
{
    int is_ok = -1;

    // TODO: To reduce the packet size, the bit size of T must be reduced. 8 * 16 = 128
    if (GCM4LEA_set_init_params(&gcm_TX, K, 128, A, 16, 32))
    {
        return is_ok; // Error
    }
    if (GCM4LEA_set_init_params(&gcm_RX, K, 128, A, 16, 32))
    {
        return is_ok; // Error
    }

    is_ok = 0;

    return is_ok;
}

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
    memcpy((uint8_t *)data, gcm_TX.T, 4);
    memcpy((uint8_t *)data + 4, gcm_TX.CC, 16);

    is_ok = 0;

    return is_ok;
}

int GCM::decrypt(OTA_Packet_s *otaPktPtr, const uint8_t *data, uint8_t dataLen) // data --> otaPktPtr
{
    int is_ok = -1;

    if (GCM4LEA_set_dec_params(&gcm_RX, data + 4, 16, N, 12, data))
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
