#pragma once

#include <Arduino.h>

// #define DBGLN(x) Serial.println(x)
#define DBGLN(x)

// #include "mbedtls/build_info.h"
#include "mbedtls/bignum.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/platform.h"
#include "mbedtls/rsa.h"

#define KEY_SIZE 1024
#define EXPONENT 65537

class RSA
{
public:
    RSA() {
        init();
    }

    ~RSA() {
        deinit();
    }

    void init();
    void deinit();
    int generate_key(const char *pers, size_t pers_len);
    int export_pubkey(unsigned char *pubkey, size_t *pubkey_len);
    int encrypt(const unsigned char *input, size_t len, unsigned char *output);
    static int encrypt(const unsigned char *pubkey, size_t pubkey_len, const unsigned char *input, size_t len, unsigned char *output);
    int decrypt(const unsigned char *ciphertext, unsigned char *decryptedtext, size_t *olen, size_t max_len);

private:
    mbedtls_rsa_context rsa_;
    mbedtls_ctr_drbg_context ctr_drbg_;
    mbedtls_entropy_context entropy_;
    const char *pers_ = "rsa_genkey";

    unsigned char pubkey_[1024] = {0};
};
