#include "ccm.h"
#include "OTA.h"

#include <fstream>

CCM::CCM()
{
}

int CCM::init()
{
    int is_ok = -1;

    if (CCM4LEA_set_init_params(&ccm_TX, K, 128, A, 0, 16))
    {
        return is_ok; // Error
    }
    if (CCM4LEA_set_init_params(&ccm_RX, K, 128, A, 0, 16))
    {
        return is_ok; // Error
    }

    is_ok = 0;

    return is_ok;
}

void CCM::setKey(uint8_t *key)
{
    memcpy(K, key, 16);
}

void CCM::setNonce(uint8_t *nonce)
{
    memcpy(N, nonce, 16);
}

int CCM::encrypt(OTA_Packet_s *otaPktPtr, uint8_t *data, uint8_t dataLen) // ota to data
{
    int is_ok = -1;

    if (CCM4LEA_set_enc_params(&ccm_TX, (uint8_t *)otaPktPtr, 16, N, 12))
    {
        return is_ok; // Error
    }
    if (CCM4LEA_enc(&ccm_TX))
    {
        return is_ok; // Error
    }

    memcpy((uint8_t *)data, ccm_TX.T, 16);
    memcpy((uint8_t *)data + 16, ccm_TX.CC, 16);

    is_ok = 0;

    return is_ok;
}

int CCM::decrypt(OTA_Packet_s *otaPktPtr, const uint8_t *data, uint8_t dataLen) // data --> otaPktPtr
{
    int is_ok = -1;

    if (CCM4LEA_set_dec_params(&ccm_RX, data + 16, 16, N, 12, data))
    {
        return is_ok; // Error
    }
    if (CCM4LEA_dec(&ccm_RX))
    {
        return is_ok; // Error
    }

    // otaPktPtr = (OTA_Packet_s *)ccm_RX.PP;
    memcpy((uint8_t *)otaPktPtr, (uint8_t *)ccm_RX.PP, ccm_RX.PP_byte_length);

    is_ok = 0;

    return is_ok;
}
