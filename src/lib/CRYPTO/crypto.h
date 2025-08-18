#pragma once

#include "OTA.h"
#include <cstdint>
#include <stddef.h>

/**
 * @brief Abstract base class for cryptographic operations
 *
 * This interface defines the common operations that all cryptographic
 * implementations should support, providing a unified API for encryption,
 * decryption, and key management.
 *
 * Written by: Donghee Park (DRONEMAP)
 */
class Crypto {
public:
    virtual ~Crypto() = default;

    /**
     * @brief Initialize the crypto instance with default parameters
     * @return 0 on success, negative value on error
     */
    virtual int init() = 0;

    /**
     * @brief Initialize the crypto instance with custom parameters
     * @param[in] K_ Key buffer
     * @param[in] K_len_ Key length
     * @param[in] A_ Associated data buffer
     * @param[in] A_len_ Associated data length
     * @param[in] N_ Nonce buffer
     * @param[in] N_len_ Nonce length, LEA use Bits, Ascon use Bytes
     * @return 0 on success, negative value on error
     */
    virtual int init(const uint8_t* K_, uint32_t K_len_,
                     const uint8_t* A_, uint32_t A_len_,
                     const uint8_t *N_, size_t N_len_) = 0;

    /**
     * @brief Encrypt plaintext data
     * @param[in] plaintext Input plaintext buffer
     * @param[in] plaintext_len Length of plaintext
     * @param[out] ciphertext Output ciphertext buffer
     * @return Length of encrypted data on success, negative value on error
     */
    virtual int encrypt(const uint8_t *plaintext, int plaintext_len, uint8_t *ciphertext) = 0;

    /**
     * @brief Decrypt ciphertext data
     * @param[in] ciphertext Input ciphertext buffer
     * @param[in] ciphertext_len Length of ciphertext
     * @param[out] plaintext Output plaintext buffer
     * @return Length of decrypted data on success, negative value on error
     */
    virtual int decrypt(const uint8_t *ciphertext, uint8_t ciphertext_len, uint8_t *plaintext) = 0;

    /**
     * @brief Encrypt data into OTA packet
     * @param[in] otaPktPtr Pointer to OTA packet structure
     * @param[out] data output data buffer
     * @param[out] dataLen Length of input data
     * @return 0 on success, negative value on error
     */
    virtual int encrypt(OTA_Packet_s *otaPktPtr, uint8_t *data, uint8_t dataLen) = 0;

    /**
     * @brief Decrypt OTA packet data
     * @param[out] otaPktPtr Pointer to OTA packet structure
     * @param[in] data input data buffer
     * @param[in] dataLen Length of output data
     * @return 0 on success, negative value on error
     */
    virtual int decrypt(OTA_Packet_s *otaPktPtr, const uint8_t *data, uint8_t dataLen) = 0;
};

