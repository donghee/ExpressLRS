#pragma once

#include "OTA.h"
#include "device.h"
#include <cstdint>

extern "C"
{
#include "gcm4lea.h"
}

#define OTA8_LEA_PACKET_SIZE (OTA8_PACKET_SIZE + 3) // NEEDS TO BE MULTIPLE OF 16
#define OTA4_LEA_PACKET_SIZE (OTA4_PACKET_SIZE + 8) // LEA unit block size is 128 bits

#define LEA_MAX_PAYLOAD_SIZE (OTA8_LEA_PACKET_SIZE * 2)

/*  LEA GCM encryption and decryption
 */

class GCM
{
private:
    GCM_st gcm_TX;
    GCM_st gcm_RX;

    // LEA-128
    uint8_t K[16] = {0x14, 0x87, 0x0B, 0x99, 0x92, 0xEA, 0x89, 0x67, 0x8A, 0x1D, 0xDF, 0xD6, 0x30, 0x91, 0x8D, 0xF0};
    uint8_t A[16] = {0, };
    // uint8_t N[12] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B};
    uint8_t N[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F }; // random 값 사용을 권장함!!!!!

    uint8_t* N_GCM = N + 4; // GCM을 위한 96-bit(12 bytes) Nonce;
    uint16_t COUNTER_TX;
    uint16_t COUNTER_RX;
    uint16_t COUNTER_RX_new;
    uint16_t COUNTER_RX_gap;
    int initStatus = 0;

    uint8_t payload[LEA_MAX_PAYLOAD_SIZE];
    uint32_t getPayloadLen(const uint8_t *data);

    void increment_nonce_counter(uint8_t *nonce);

public:
    GCM();
    int init();
    int init(uint8_t *K_, size_t K_len_, uint8_t *A_, size_t A_len_, uint8_t *N_, size_t N_len_);
    int encrypt(OTA_Packet_s *otaPktPtr, uint8_t *data, uint8_t dataLen);       // otaPktPtr -> data
    int decrypt(OTA_Packet_s *otaPktPtr, const uint8_t *data, uint8_t dataLen); // data --> otaPktPtr
    void reset();
    int counter() { return COUNTER_RX; };
};
