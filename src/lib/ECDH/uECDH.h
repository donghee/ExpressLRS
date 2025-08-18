#pragma once

#include <stddef.h>
#include <stdint.h>

extern "C" {
#include "ecdh.h"
}

/**
 * @brief Elliptic Curve Diffie-Hellman (ECDH) key exchange implementation
 *
 * This class provides a C++ wrapper for ECDH cryptographic operations,
 * enabling secure key exchange between two parties over an insecure channel.
 * ECDH is used in the ExpressLRS handshake protocol to establish shared
 * encryption keys between transmitter and receiver modules.
 *
 * Features:
 * - 256-bit elliptic curve cryptography (secp256r1/P-256)
 * - Compressed and uncompressed public key support
 * - Secure shared secret generation
 * - Dual key pair support for advanced protocols
 * - Optimized C implementation with ARM assembly support
 *
 * Security Note: This implementation uses well-established elliptic curve
 * parameters and provides forward secrecy when used properly.
 *
 * Written by: Donghee Park (DRONEMAP)
 */
class ECDH {
public:
    /**
     * @brief Default constructor for ECDH key exchange engine
     */
    ECDH();

    /**
     * @brief Destructor - clears sensitive key material
     */
    ~ECDH();

    /**
     * @brief Initialize the ECDH instance with default curve parameters
     */
    void init();

    /**
     * @brief Compress another party's public key (for testing purposes)
     * @param[in] public_key Input uncompressed public key buffer
     * @param[in] public_key_len Length of input public key (should be 64 bytes)
     * @param[out] compressed_pubkey Output compressed public key buffer
     * @param[out] compressed_pubkey_len Output compressed key length
     * @return 0 on success, negative value on error
     */
    int compress_other_public_key(const char* public_key, size_t public_key_len, unsigned char *compressed_pubkey, size_t *compressed_pubkey_len);

    /**
     * @brief Compress own public key to reduce transmission overhead
     * @param[out] compressed_pubkey Output compressed public key buffer
     * @param[out] compressed_pubkey_len Output compressed key length
     * @return 0 on success, negative value on error
     */
    int compress_public_key(unsigned char *compressed_pubkey, size_t *compressed_pubkey_len);

    /**
     * @brief Export own public key for transmission to other party
     * @param[out] pubkey Output public key buffer (64 bytes for uncompressed)
     * @param[in,out] pubkey_len Input: buffer size, Output: actual key length
     * @return 0 on success, negative value on error
     */
    int get_public_key(unsigned char *pubkey, size_t *pubkey_len);

    /**
     * @brief Generate shared secret using other party's compressed public key
     * @param[in] compressed_pubkey Other party's compressed public key
     * @param[in] compressed_pubkey_len Length of compressed public key
     * @param[in] curves Elliptic curve parameters to use
     * @return 0 on success, negative value on error
     */
    int generate_other_secret_key(const char *compressed_pubkey, size_t compressed_pubkey_len, const struct ECC_Curve_t *curves);

    /**
     * @brief Generate shared secret from other party's compressed public key
     * @param[in] compressed_public_key Other party's compressed public key
     * @param[in] compressed_public_key_len Length of compressed public key
     * @return 0 on success, negative value on error
     */
    int generate_secret_key_from_compressed_public_key(const char *compressed_public_key, size_t compressed_public_key_len);

    /**
     * @brief Generate shared secret using other party's uncompressed public key
     * @param[in] public_key_2 Other party's uncompressed public key (64 bytes)
     * @param[in] public_key_2_len Length of public key (should be 64)
     * @return 0 on success, negative value on error
     */
    int generate_secret_key(const char *public_key_2, size_t public_key_2_len);

    /**
     * @brief Export the computed shared secret key
     * @param[out] secret_key Output buffer for shared secret (32 bytes)
     * @param[out] secret_key_len Input: buffer size, Output secret length
     * @return 0 on success, negative value on error
     */
    int export_secret_key(unsigned char *secret_key, size_t *secret_key_len);

private:
    /** @brief Pointer to elliptic curve parameters for own key pair */
    const struct ECC_Curve_t *curves_;

    /** @brief Pointer to elliptic curve parameters for other party's key pair */
    const struct ECC_Curve_t *other_curves_;

    /** @brief Own private key (256-bit/32 bytes) */
    uint8_t private_key_[32] = { 0, };

    /** @brief Own public key (512-bit/64 bytes, uncompressed format) */
    uint8_t public_key_[64] = { 0, };

    /** @brief Own compressed public key buffer (up to 64 bytes) */
    uint8_t compressed_public_key_[64] = { 0, };

    /** @brief Computed shared secret key (256-bit/32 bytes) */
    uint8_t secret_key_[32] = { 0, };

    /** @brief Secondary private key for testing protocols (256-bit/32 bytes) */
    uint8_t private_key2_[32] = { 0, };

    /** @brief Secondary public key for testing protocols (512-bit/64 bytes) */
    uint8_t public_key2_[64] = { 0, };

    /** @brief Secondary shared secret for testing protocols (256-bit/32 bytes) */
    uint8_t secret_key2_[32] = { 0, };
};
