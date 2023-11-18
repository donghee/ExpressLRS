#pragma once

#include "OTA.h"
#include "device.h"
#include <cstdint>
extern "C"
{
#include "ccm4lea.h"
}

#define USE_LEA
#define OTA8_LEA_PACKET_SIZE (OTA8_PACKET_SIZE + 3) // NEEDS TO BE MULTIPLE OF 16
#define OTA4_LEA_PACKET_SIZE (OTA4_PACKET_SIZE + 8) // LEA unit block size is 128 bits
#define LEA_MAX_PAYLOAD_SIZE (OTA8_LEA_PACKET_SIZE * 2)

/*  LEA CCM encryption and decryption
 */

class CCM
{
private:
    CCM_st ccm_TX;
    CCM_st ccm_RX;

    // LEA-128
    uint8_t K[16] = {0x14, 0x87, 0x0B, 0x99, 0x92, 0xEA, 0x89, 0x67, 0x8A, 0x1D, 0xDF, 0xD6, 0x30, 0x91, 0x8D, 0xF0};
    uint8_t A[16] = {
        0,
    };
    uint8_t N[12] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B};

    uint8_t payload[LEA_MAX_PAYLOAD_SIZE];
    uint32_t getPayloadLen(const uint8_t *data);

public:
    CCM();
    int init();
    void setKey(uint8_t *key);
    void setNonce(uint8_t *nonce);
    int encrypt(OTA_Packet_s *otaPktPtr, uint8_t *data, uint8_t dataLen);       // otaPktPtr -> data
    int decrypt(OTA_Packet_s *otaPktPtr, const uint8_t *data, uint8_t dataLen); // data --> otaPktPtr
    void reset();
};
