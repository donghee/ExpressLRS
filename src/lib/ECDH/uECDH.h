#pragma once

#include <stddef.h>
#include <stdint.h>

extern "C" {
#include "ecdh.h"
}

class ECDH {
public:
    ECDH();
    ~ECDH();

    void init();
    int compress_other_public_key(const char* public_key, size_t public_key_len, unsigned char *compressed_pubkey, size_t *compressed_pubkey_len); // for testing
    int compress_public_key(unsigned char *compressed_pubkey, size_t *compressed_pubkey_len);
    int get_public_key(unsigned char *pubkey, size_t *pubkey_len);
    int generate_other_secret_key(const char *compressed_pubkey, size_t compressed_pubkey_len, const struct ECC_Curve_t *curves);
    int generate_secret_key_from_compressed_public_key(const char *compressed_public_key, size_t compressed_public_key_len);
    int generate_secret_key(const char *public_key_2, size_t public_key_2_len);
    int export_secret_key(unsigned char *secret_key, size_t *secret_key_len);

private:
	const struct ECC_Curve_t *curves_;
	const struct ECC_Curve_t *other_curves_;

	uint8_t private_key_[32] = { 0, };
	uint8_t public_key_[64] = { 0, };

	uint8_t compressed_public_key_[64] = { 0, };
	uint8_t secret_key_[32] =	{ 0, };

	uint8_t private_key2_[32] = { 0, };
	uint8_t public_key2_[64] = { 0, };
	uint8_t secret_key2_[32] =	{ 0, };
};
