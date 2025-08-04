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
     * @param K_ Key buffer
     * @param K_len_ Key length
     * @param A_ Associated data buffer
     * @param A_len_ Associated data length
     * @param N_ Nonce buffer
     * @param N_len_ Nonce length, LEA use Bits, Ascon use Bytes
     * @return 0 on success, negative value on error
j    */
    virtual int init(const uint8_t* K_, uint32_t K_len_,
                     const uint8_t* A_, uint32_t A_len_,
                     uint8_t *N_, size_t N_len_) = 0;

    /**
     * @brief Encrypt plaintext data
     * @param plaintext Input plaintext buffer
     * @param plaintext_len Length of plaintext
     * @param ciphertext Output ciphertext buffer
     * @return Length of encrypted data on success, negative value on error
     */
    virtual int encrypt(const uint8_t *plaintext, int plaintext_len, uint8_t *ciphertext) = 0;

    /**
     * @brief Decrypt ciphertext data
     * @param ciphertext Input ciphertext buffer
     * @param ciphertext_len Length of ciphertext
     * @param plaintext Output plaintext buffer
     * @return Length of decrypted data on success, negative value on error
     */
    virtual int decrypt(const uint8_t *ciphertext, uint8_t ciphertext_len, uint8_t *plaintext) = 0;

    /**
     * @brief Encrypt data into OTA packet
     * @param otaPktPtr Pointer to OTA packet structure
     * @param data output data buffer
     * @param dataLen Length of input data
     * @return 0 on success, negative value on error
     */
    virtual int encrypt(OTA_Packet_s *otaPktPtr, uint8_t *data, uint8_t dataLen) = 0;

    /**
     * @brief Decrypt OTA packet data
     * @param otaPktPtr Pointer to OTA packet structure
     * @param data input data buffer
     * @param dataLen Length of output data
     * @return 0 on success, negative value on error
     */
    virtual int decrypt(OTA_Packet_s *otaPktPtr, const uint8_t *data, uint8_t dataLen) = 0;

    /**
     * @brief Increment nonce counter by 1
     * @param nonce Nonce buffer to increment
     */
    virtual void increment_nonce_counter(uint8_t *nonce) = 0;

    /**
     * @brief Increase nonce counter by specified increment (up to 32-bit)
     * @param nonce Nonce buffer to increment
     * @param increment Value to add to the counter
     */
    virtual void increase_nonce_counter_up_to_32bits_increment(uint8_t *nonce, uint32_t increment) = 0;

    /**
     * @brief Get current counter value
     * @return Current counter value
     */
    virtual int counter() = 0;
};

