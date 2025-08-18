#pragma once

#include "OTA.h"
#include <cstdint>
#include "crypto.h"

extern "C"
{
#include "ascon128x.h"
#include <stddef.h>
}

/**
 * @brief ASCON-128 lightweight authenticated encryption implementation
 *
 * This class implements the ASCON-128 authenticated encryption algorithm,
 * providing secure encryption and decryption capabilities with built-in
 * authentication. ASCON is a lightweight cryptographic algorithm designed
 * for resource-constrained environments.
 *
 * Features:
 * - 128-bit key and 128-bit nonce
 * - Authenticated encryption with associated data (AEAD)
 * - Counter-based nonce management for TX/RX operations
 * - OTA packet encryption/decryption support
 *
 * Written by: Donghee Park (DRONEMAP)
 */
class Ascon128 : public Crypto
{
private:
    /** @brief ASCON state for transmission operations */
    ASCON_st ascon_TX;

    /** @brief ASCON state for reception operations */
    ASCON_st ascon_RX;

    /** @brief 128-bit encryption key (fixed for this implementation) */
    uint8_t K[16] = {0x14, 0x87, 0x0B, 0x99, 0x92, 0xEA, 0x89, 0x67, 0x8A, 0x1D, 0xDF, 0xD6, 0x30, 0x91, 0x8D, 0xF0};

    /** @brief 128-bit associated data buffer (initialized to zeros) */
    uint8_t A[16] = {0, };

    /** @brief 128-bit nonce buffer (NOTE: Should use random values in production!) */
    uint8_t N[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };

    /** @brief 128-bit authentication tag buffer */
    uint8_t T[16] = {0, };

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

    /** @brief 4-bit counter for compact operations */
    uint8_t COUNTER_4b;

    /** @brief New 4-bit counter value */
    uint8_t COUNTER_4b_new;

    /** @brief Gap between 4-bit counters */
    uint8_t COUNTER_4b_gap;

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
     * @brief Default constructor for ASCON-128 cryptographic engine
     */
    Ascon128();

    /**
     * @brief Initialize the ASCON-128 instance with default parameters
     * @return 0 on success, negative value on error
     */
    int init();

    /**
     * @brief Initialize the ASCON-128 instance with custom parameters
     * @param[in] K_ Key buffer (must be 16 bytes for ASCON-128)
     * @param[in] K_len_ Key length in bytes (should be 16)
     * @param[in] A_ Associated data buffer
     * @param[in] A_len_ Associated data length in bytes
     * @param[in] N_ Nonce buffer (must be 16 bytes for ASCON-128)
     * @param[in] N_len_ Nonce length in bytes (should be 16)
     * @return 0 on success, negative value on error
     */
    int init(const uint8_t* K_, uint32_t K_len_,
             const uint8_t* A_, uint32_t A_len_,
             const uint8_t *N_, size_t N_len_);

    /**
     * @brief Encrypt plaintext data using ASCON-128
     * @param[in] plaintext Input plaintext buffer
     * @param[in] plaintext_len Length of plaintext in bytes
     * @param[out] ciphertext Output ciphertext buffer (must be at least plaintext_len + 16 bytes for tag)
     * @return Length of encrypted data including tag on success, negative value on error
     */
    int encrypt(const uint8_t *plaintext, int plaintext_len, uint8_t *ciphertext);

    /**
     * @brief Decrypt ciphertext data using ASCON-128
     * @param[in] ciphertext Input ciphertext buffer (includes authentication tag)
     * @param[in] ciphertext_len Length of ciphertext in bytes (including tag)
     * @param[out] plaintext Output plaintext buffer
     * @return Length of decrypted data on success, negative value on error or authentication failure
     */
    int decrypt(const uint8_t *ciphertext, uint8_t ciphertext_len, uint8_t *plaintext);

    /**
     * @brief Encrypt OTA packet data using ASCON-128
     * @param[in] otaPktPtr Pointer to OTA packet structure (input data source)
     * @param[out] data Output encrypted data buffer
     * @param[out] dataLen Length of data to encrypt
     * @return 0 on success, negative value on error
     */
    int encrypt(OTA_Packet_s *otaPktPtr, uint8_t *data, uint8_t dataLen);

    /**
     * @brief Decrypt OTA packet data using ASCON-128
     * @param[out] otaPktPtr Pointer to OTA packet structure (output data destination)
     * @param[in] data Input encrypted data buffer
     * @param[in] dataLen Length of encrypted data
     * @return 0 on success, negative value on error or authentication failure
     */
    int decrypt(OTA_Packet_s *otaPktPtr, const uint8_t *data, uint8_t dataLen);

    /**
     * @brief Get current reception counter value
     * @return Current RX counter value for debugging and synchronization purposes
     */
    int counter() { return COUNTER_RX; };
};

