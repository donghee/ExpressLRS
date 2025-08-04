#include "uECDH.h"
#include <Arduino.h>

extern HardwareSerial DebugSerial;

static int RNG_(uint8_t *obuf, size_t len)
{
    if (obuf == NULL || len == 0) {
        return 0; // Failure
    }

    static bool seed_initialized = false;
    if (!seed_initialized) {
        // Use analog noise for better entropy
        randomSeed(analogRead(0) ^ micros());
        seed_initialized = true;
    }

    // Fill buffer with random bytes
    for (size_t i = 0; i < len; i++) {
        obuf[i] = (uint8_t)random(256);
    }

    // DebugSerial.print("RNG output: ");
    // for (size_t i = 0; i < len; i++) {
    //     DebugSerial.print(obuf[i], HEX);
    // }
    // DebugSerial.println();

    return 1; // Success
}

ECDH::ECDH()
{
}

ECDH::~ECDH()
{
}

void ECDH::init()
{
    ECC_set_rng(&RNG_);

    curves_ = ECC_secp256k1();

    ECC_make_key(public_key_, private_key_, curves_);

    // DebugSerial.print("ECC private key 1: ");
    // for (size_t i = 0; i < sizeof(private_key_); i++) {
    //     DebugSerial.printf("%02x", private_key_[i]);
    // }
    // DebugSerial.println();
    //
    // DebugSerial.print("ECC public key 1: ");
    // for (size_t i = 0; i < sizeof(public_key_); i++) {
    //     DebugSerial.printf("%02x", public_key_[i]);
    // }
    // DebugSerial.println();
    //
    // uint8_t compressed_public_key1_[64] = { 0, };
    // size_t compressed_public_key1_len = 0;
    //
    // compress_public_key((unsigned char *)compressed_public_key1_, &compressed_public_key1_len);
    // DebugSerial.print("ECC compressed public key 1: ");
    // for (size_t i = 0; i < compressed_public_key1_len; i++) {
    //     DebugSerial.printf("%02x", compressed_public_key1_[i]);
    // }
    // DebugSerial.println();
    //
    // DebugSerial.println("---");
    // // Generate a second key pair for demonstration
    // other_curves_ = ECC_secp256k1();
    // ECC_make_key(public_key2_, private_key2_, other_curves_);
    //
    // DebugSerial.print("ECC private key 2: ");
    // for (size_t i = 0; i < sizeof(private_key2_); i++) {
    //     DebugSerial.printf("%02x", private_key2_[i]);
    // }
    // DebugSerial.println();
    //
    // DebugSerial.print("ECC public key 2: ");
    // for (size_t i = 0; i < sizeof(public_key2_); i++) {
    //     DebugSerial.printf("%02x", public_key2_[i]);
    // }
    // DebugSerial.println();
    //
    // uint8_t compressed_public_key2_[64] = { 0, };
    // size_t compressed_public_key2_len = 0;
    // compress_other_public_key((const char *)public_key2_, sizeof(public_key2_), (unsigned char *)compressed_public_key2_, &compressed_public_key2_len);
    // generate_secret_key_from_compressed_public_key((const char *)compressed_public_key2_, compressed_public_key2_len);
    //
    // DebugSerial.println("---");
    // // Generate shared secret keys
    // DebugSerial.print("ECC security key 1: ");
    // for (size_t i = 0; i < sizeof(secret_key_); i++) {
    //     DebugSerial.printf("%02x", secret_key_[i]);
    // }
    // DebugSerial.println();
    //
    // ECC_shared_secret(public_key_, private_key2_, secret_key2_, other_curves_);
    // DebugSerial.print("ECC security key 2: ");
    // for (size_t i = 0; i < sizeof(secret_key2_); i++) {
    //     DebugSerial.printf("%02x", secret_key2_[i]);
    // }
    // DebugSerial.println();
    // DebugSerial.println("---");
    //
    // memset(secret_key_, 0, sizeof(secret_key_));
    // memset(secret_key2_, 0, sizeof(secret_key2_));
}

int ECDH::compress_public_key(unsigned char *to_compressed_public_key, size_t *to_compressed_public_key_len)
{
    if (to_compressed_public_key == NULL || to_compressed_public_key_len == NULL) {
        return 0; // Failure
    }

    ECC_compress_public_key(public_key_, to_compressed_public_key, curves_);

    *to_compressed_public_key_len = 32; // or curves_->num_bytes;

    DebugSerial.print("!Origin public key ");
    for (size_t i = 0; i < 64; i++) {
        DebugSerial.printf("%02x", public_key_[i]);
    }
    DebugSerial.println();

    DebugSerial.print("!Compress public key ");
    for (size_t i = 0; i < 32; i++) {
        DebugSerial.printf("%02x", to_compressed_public_key[i]);
    }
    DebugSerial.println();

    DebugSerial.print("!DeCompress public key ");
    uint8_t decompressed_public_key[64] = { 0, };
    ECC_decompress_public_key(to_compressed_public_key, decompressed_public_key, curves_);
    for (size_t i = 0; i < 64; i++) {
        DebugSerial.printf("%02x", decompressed_public_key[i]);
    }
    DebugSerial.println();

    return 1; // Success
}

int ECDH::get_public_key(unsigned char *pubkey, size_t *pubkey_len)
{
    if (pubkey == NULL || pubkey_len == NULL) {
        return 0; // Failure
    }

    memcpy(pubkey, public_key_, sizeof(public_key_));
    *pubkey_len = sizeof(public_key_); // 64

    DebugSerial.print("!Public key: ");
    for (size_t i = 0; i < sizeof(public_key_); i++) {
        DebugSerial.printf("%02x", public_key_[i]);
    }
    DebugSerial.println();

    return 1; // Success
}

int ECDH::compress_other_public_key(const char* from_public_key, size_t from_public_key_len, unsigned char *to_compressed_public_key, size_t *to_compressed_public_key_len)
{
    if (to_compressed_public_key == NULL || to_compressed_public_key_len == NULL) {
        return 0; // Failure
    }

    ECC_compress_public_key((uint8_t *)from_public_key, to_compressed_public_key, other_curves_);
    *to_compressed_public_key_len = 32;
    return 1; // Success
}

int ECDH::generate_secret_key(const char *public_key_2, size_t public_key_2_len)
{
    int ret = 1;
    DebugSerial.print("!Other public key: ");
    for (size_t i = 0; i < 64; i++) {
        DebugSerial.printf("%02x", (uint8_t)public_key_2[i]);
    }
    DebugSerial.println();
    ret = ECC_shared_secret((const uint8_t *)public_key_2, private_key_, secret_key_, curves_);
    memcpy(public_key2_, public_key_2, 64);
    return ret;
}

int ECDH::generate_secret_key_from_compressed_public_key(const char *compressed_public_key, size_t compressed_public_key_len)
{
    int ret = 1;
	uint8_t public_key_2[64] = { 0, };
    DebugSerial.print("!Compressed other public key: ");
    for (size_t i = 0; i < 32; i++) {
        DebugSerial.printf("%02x", (uint8_t)compressed_public_key[i]);
    }
    DebugSerial.println();
    ECC_decompress_public_key((const uint8_t *)compressed_public_key, public_key_2, curves_);
    ret = ECC_shared_secret((const uint8_t *)public_key_2, private_key_, secret_key_, curves_);
    // memcpy(public_key2_, public_key_2, sizeof(public_key_2));
    memcpy(public_key2_, public_key_2, 64);
    return ret;
}

int ECDH::generate_other_secret_key(const char *compressed_public_key, size_t compressed_public_key_len, const struct ECC_Curve_t *curves)
{
    int ret = 1;
	uint8_t public_key_2[64] = { 0, };
    ECC_decompress_public_key((const uint8_t *)compressed_public_key, public_key_2, curves);
    ret = ECC_shared_secret((const uint8_t *)public_key_2, private_key_, secret_key_, curves);
    memcpy(public_key2_, public_key_2, sizeof(public_key_2));
    return ret;
}

int ECDH::export_secret_key(unsigned char *secret_key, size_t *secret_key_len)
{
    if (secret_key == NULL || secret_key_len == NULL) {
        return 0; // Failure
    }

    DebugSerial.print("\r\nprivate key: ");
    for (size_t i = 0; i < sizeof(private_key_); i++) {
      DebugSerial.printf("%02x", private_key_[i]);
    }
    DebugSerial.println();

    DebugSerial.print("\r\npublic key: ");
    for (size_t i = 0; i < sizeof(public_key_); i++) {
      DebugSerial.printf("%02x", public_key_[i]);
    }
    DebugSerial.println();

    DebugSerial.print("\r\nother public key: ");
    for (size_t i = 0; i < sizeof(public_key2_); i++) {
      DebugSerial.printf("%02x", public_key2_[i]);
    }
    DebugSerial.println();

    DebugSerial.print("\r\nsecret key: ");
    for (size_t i = 0; i < sizeof(secret_key_); i++) {
      DebugSerial.printf("%02x", secret_key_[i]);
    }
    DebugSerial.println();

    memcpy(secret_key, secret_key_, sizeof(secret_key_));
    *secret_key_len = sizeof(secret_key_);
    return 1; // Success
}
