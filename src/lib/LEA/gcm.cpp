#include "gcm.h"
#include "OTA.h"

#include <fstream>

GCM::GCM()
{
}

int GCM::init()
{
    int is_ok = -1;

    if (GCM4LEA_set_init_params(&gcm_TX, K, 128, A, 0, 128))
    {
        return is_ok; // Error
    }
    if (GCM4LEA_set_init_params(&gcm_RX, K, 128, A, 0, 128))
    {
        return is_ok; // Error
    }

    is_ok = 0;

    return is_ok;
}

void GCM::setKey(uint8_t *key)
{
    memcpy(K, key, 16);
}

void GCM::setNonce(uint8_t *nonce)
{
    memcpy(N, nonce, 16);
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

    memcpy((uint8_t *)data, gcm_TX.T, 16);
    memcpy((uint8_t *)data + 16, gcm_TX.CC, 16);

    is_ok = 0;

    return is_ok;
}

int GCM::decrypt(OTA_Packet_s *otaPktPtr, const uint8_t *data, uint8_t dataLen) // data --> otaPktPtr
{
    int is_ok = -1;

    if (GCM4LEA_set_dec_params(&gcm_RX, data + 16, 16, N, 12, data))
    {
        return is_ok; // Error
    }
    if (GCM4LEA_dec(&gcm_RX))
    {
        return is_ok; // Error
    }

    // otaPktPtr = (OTA_Packet_s *)gcm_RX.PP;
    memcpy((uint8_t *)otaPktPtr, (uint8_t *)gcm_RX.PP, gcm_RX.PP_byte_length);

    is_ok = 0;

    return is_ok;
}
