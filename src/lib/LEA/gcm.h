#pragma once

#include "OTA.h"
#include "device.h"
#include <cstdint>
#include "crypto.h"

extern "C"
{
#include "gcm4lea.h"
}

/** @brief OTA packet size for 8-bit mode with LEA encryption (must be multiple of 16) */
#define OTA8_LEA_PACKET_SIZE (OTA8_PACKET_SIZE + 3)

/** @brief OTA packet size for 4-bit mode with LEA encryption (LEA block size is 128 bits) */
#define OTA4_LEA_PACKET_SIZE (OTA4_PACKET_SIZE + 8)

/** @brief Maximum payload size for LEA encryption operations */
#define LEA_MAX_PAYLOAD_SIZE (OTA8_LEA_PACKET_SIZE * 2)

/** @brief Additional packet size for LEA (2 bytes counter + 2 bytes tag) */
#define LEA_ADD_PACKET_SIZE 4

/**
 * @brief LEA-GCM authenticated encryption implementation
 *
 * This class implements the LEA (Lightweight Encryption Algorithm) with GCM
 * (Galois/Counter Mode) for authenticated encryption. LEA is a Korean national
 * standard block cipher designed for high performance in both software and
 * hardware implementations.
 *
 * Features:
 * - 128-bit LEA block cipher with GCM mode
 * - 128-bit key and 96-bit nonce (GCM standard)
 * - Authenticated encryption with associated data (AEAD)
 * - Counter-based nonce management for TX/RX operations
 * - OTA packet encryption/decryption support
 * - Optimized for ExpressLRS radio control systems
 *
 * Written by: Donghee Park (DRONEMAP)
 */
class GCM : public Crypto
{
private:
    /** @brief GCM state for transmission operations */
    GCM_st gcm_TX;

    /** @brief GCM state for reception operations */
    GCM_st gcm_RX;

    /** @brief 128-bit LEA encryption key (fixed for this implementation) */
    uint8_t K[16] = {0x14, 0x87, 0x0B, 0x99, 0x92, 0xEA, 0x89, 0x67, 0x8A, 0x1D, 0xDF, 0xD6, 0x30, 0x91, 0x8D, 0xF0};

    /** @brief 128-bit associated data buffer (initialized to zeros) */
    uint8_t A[16] = {0, };

    /** @brief 128-bit nonce buffer (NOTE: Should use random values in production!) */
    uint8_t N[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };

    /** @brief Pointer to 96-bit GCM nonce (12 bytes, offset by 4 bytes from N) */
    uint8_t* N_GCM = N + 4;

    /** @brief 16-bit counter for transmission operations */
    uint16_t COUNTER_TX;

    /** @brief 16-bit counter for reception operations */
    uint16_t COUNTER_RX;

    /** @brief New reception counter value for synchronization */
    uint16_t COUNTER_RX_new;

    /** @brief Gap between reception counters for replay protection */
    uint16_t COUNTER_RX_gap;

    /** @brief Initialization status flag (0 = not initialized, 1 = initialized) */
    int initStatus = 0;

    /** @brief Payload buffer for encryption/decryption operations */
    uint8_t payload[LEA_MAX_PAYLOAD_SIZE];

    /**
     * @brief Get payload length from encrypted data
     * @param[in] data Input data buffer to analyze
     * @return Length of payload in bytes
     */
    uint32_t getPayloadLen(const uint8_t *data);

    /**
     * @brief Increment nonce counter by 1
     * @param[in,out] nonce Nonce buffer to increment
     */
    void increment_nonce_counter(uint8_t *nonce);

    /**
     * @brief Increase nonce counter by specified increment (up to 32-bit)
     * @param[in,out] nonce Nonce buffer to increment
     * @param[in] increment Value to add to the counter
     */
    void increase_nonce_counter_up_to_32bits_increment(uint8_t *nonce, uint32_t increment);

public:
    /**
     * @brief Default constructor for LEA-GCM cryptographic engine
     */
    GCM();

    /**
     * @brief Initialize the LEA-GCM instance with default parameters
     * @return 0 on success, negative value on error
     */
    int init();

    /**
     * @brief Initialize the LEA-GCM instance with custom parameters
     * @param[in] K_ Key buffer (must be 16 bytes for LEA-128)
     * @param[in] K_len_ Key length in bytes (should be 16)
     * @param[in] A_ Associated data buffer
     * @param[in] A_len_ Associated data length in bytes
     * @param[in] N_ Nonce buffer (first 12 bytes used for GCM)
     * @param[in] N_len_ Nonce length in bytes
     * @return 0 on success, negative value on error
     */
    int init(const uint8_t* K_, uint32_t K_len_, const uint8_t* A_, uint32_t A_len_, const uint8_t *N_, size_t N_len_);

    /**
     * @brief Encrypt plaintext data using LEA-GCM
     * @param[in] plaintext Input plaintext buffer
     * @param[in] plaintext_len Length of plaintext in bytes
     * @param[out] ciphertext Output ciphertext buffer (must be at least plaintext_len + 16 bytes for tag)
     * @return Length of encrypted data including tag on success, negative value on error
     */
    int encrypt(const uint8_t *plaintext, int plaintext_len, uint8_t *ciphertext);

    /**
     * @brief Decrypt ciphertext data using LEA-GCM
     * @param[in] ciphertext Input ciphertext buffer (includes authentication tag)
     * @param[in] ciphertext_len Length of ciphertext in bytes (including tag)
     * @param[out] plaintext Output plaintext buffer
     * @return Length of decrypted data on success, negative value on error or authentication failure
     */
    int decrypt(const uint8_t *ciphertext, uint8_t ciphertext_len, uint8_t *plaintext);

    /**
     * @brief Encrypt OTA packet data using LEA-GCM
     * @param[in] otaPktPtr Pointer to OTA packet structure (input data source)
     * @param[out] data Output encrypted data buffer
     * @param[out] dataLen Length of data to encrypt
     * @return 0 on success, negative value on error
     */
    int encrypt(OTA_Packet_s *otaPktPtr, uint8_t *data, uint8_t dataLen);

    /**
     * @brief Decrypt OTA packet data using LEA-GCM
     * @param[out] otaPktPtr Pointer to OTA packet structure (output data destination)
     * @param[in] data Input encrypted data buffer
     * @param[in] dataLen Length of encrypted data
     * @return 0 on success, negative value on error or authentication failure
     */
    int decrypt(OTA_Packet_s *otaPktPtr, const uint8_t *data, uint8_t dataLen);

    /**
     * @brief Reset the GCM instance to initial state
     */
    void reset();

    /**
     * @brief Get current reception counter value
     * @return Current RX counter value for debugging and synchronization purposes
     */
    int counter() { return COUNTER_RX; };
};
