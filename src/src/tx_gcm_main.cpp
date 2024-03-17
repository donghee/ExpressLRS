#include <Arduino.h>
#include "targets.h"
#include "SX1280Driver.h"
#include "common.h"
#include "FHSS.h"
#include "gcm.h"

//#include "mbedtls/build_info.h"
#include "mbedtls/platform.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/bignum.h"
#include "mbedtls/rsa.h"

#define KEY_SIZE 1024
#define EXPONENT 65537

#define DATA_SIZE 32

SX12XX_Radio_Number_t transmittingRadio = Radio.GetLastSuccessfulPacketRadio();
GCM lea_gcm;

WORD_ALIGNED_ATTR OTA_Packet_s otaPkt = {0};
uint8_t ciphertext[DATA_SIZE] = { 0 };
uint8_t testtext[DATA_SIZE] = { 0 };

void ICACHE_RAM_ATTR TXdoneCallback()
{
  int ret = 0;
  //  delayMicroseconds(100);
  memcpy(testtext, &otaPkt, sizeof(otaPkt));

  ret = lea_gcm.encrypt(&otaPkt, ciphertext, 32);
  if (ret != 0) {
    DBGLN("LEA GCM encrypt error");
    return;
  }

  uint8_t plaintext[DATA_SIZE];
  ret = lea_gcm.decrypt((OTA_Packet_s *) plaintext, (const uint8_t *) ciphertext, 32);
  if (ret != 0) {
    DBGLN("LEA GCM decrypt error");
  }

  Radio.TXnb(ciphertext, sizeof(ciphertext), transmittingRadio);
}

bool ICACHE_RAM_ATTR RXdoneCallback(SX12xxDriverCommon::rx_status const status)
{
  for (int i = 0; i < 8; i++)
  {
    Serial.print(Radio.RXdataBuffer[i], HEX);
    Serial.print(",");
  }
  Serial.println("");
  //Radio.RXnb();
  return true;
}

unsigned char pubkey[1024] = {0};

void rsa_init(mbedtls_rsa_context *rsa, mbedtls_ctr_drbg_context *ctr_drbg)
{
    mbedtls_ctr_drbg_init( ctr_drbg );
    mbedtls_rsa_init( rsa, MBEDTLS_RSA_PKCS_V15, 0 );

    return;
}

int rsa_generate_key(mbedtls_rsa_context *rsa, mbedtls_ctr_drbg_context *ctr_drbg, const char *pers, size_t pers_len)
{
    int ret = 1;
    int exit_code = MBEDTLS_EXIT_FAILURE;
    mbedtls_entropy_context entropy;

    mbedtls_entropy_init( &entropy );
    if( ( ret = mbedtls_ctr_drbg_seed( ctr_drbg, mbedtls_entropy_func, &entropy,
                               (const unsigned char *) pers,
                               pers_len) ) != 0 )
    {
        //Serial.println( " failed\n  ! mbedtls_ctr_drbg_seed returned %d\n", ret );
        DBGLN("SEED");
    }

    if( ( ret = mbedtls_rsa_gen_key( rsa, mbedtls_ctr_drbg_random, ctr_drbg, KEY_SIZE,
                                     EXPONENT ) ) != 0 )
    {
        //Serial.println( " failed\n  ! mbedtls_rsa_gen_key returned %d\n\n", ret );
        DBGLN("GEN KEY");
    }

    mbedtls_entropy_free( &entropy );
    return ret;
}

int rsa_export_pubkey(mbedtls_rsa_context *rsa, unsigned char *pubkey)
{
    int ret = 1;
    int exit_code = MBEDTLS_EXIT_FAILURE;

    mbedtls_mpi N, E;
    mbedtls_mpi_init( &N ); mbedtls_mpi_init( &E );

    if( ( ret = mbedtls_rsa_export( rsa, &N, NULL, NULL, NULL, &E ) ) != 0 )
    {
        //mbedtls_printf( " failed\n  ! could not export RSA parameters\n\n" );
        DBGLN("EXPORT");
        return ret;
    }

    // save public key
    mbedtls_mpi_write_binary( &N, pubkey, rsa->len );
    mbedtls_mpi_write_binary( &E, pubkey+rsa->len, rsa->len );

    mbedtls_mpi_free( &N ); mbedtls_mpi_free( &E );

    return ret;
}

int rsa_encrypt(mbedtls_rsa_context *rsa, mbedtls_ctr_drbg_context *ctr_drbg, const unsigned char *input, unsigned char *output, size_t len)
{
    int ret = 1;
    int exit_code = MBEDTLS_EXIT_FAILURE;

    ret = mbedtls_rsa_pkcs1_encrypt( rsa, mbedtls_ctr_drbg_random,
                                     ctr_drbg, MBEDTLS_RSA_PUBLIC,
                                     len, input, output );

    if( ret != 0 )
    {
        DBGLN("FAILED ENCRYPT");
    }

    return ret;
}

int rsa_encrypt(const unsigned char *pubkey, size_t key_len, const unsigned char *input, unsigned char *output, size_t len)
{
    int ret = 1;
    int exit_code = MBEDTLS_EXIT_FAILURE;

    mbedtls_rsa_context rsa_pub;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_mpi N_pub, E_pub;
    const char *pers = "rsa_genkey";

    int pubkey_error = 0;
    mbedtls_ctr_drbg_init( &ctr_drbg );
    mbedtls_rsa_init( &rsa_pub, MBEDTLS_RSA_PKCS_V15, 0 );
    mbedtls_mpi_init( &N_pub ); mbedtls_mpi_init( &E_pub );

    mbedtls_entropy_init( &entropy );
    if( ( ret = mbedtls_ctr_drbg_seed( &ctr_drbg, mbedtls_entropy_func, &entropy,
                               (const unsigned char *) pers,
                               strlen( pers ) ) ) != 0 )
    {
        DBGLN("SEED");
    }

    // load binary public key
    if( ( ret = mbedtls_mpi_read_binary( &N_pub, pubkey, key_len ) ) != 0  ||
        ( ret = mbedtls_mpi_read_binary( &E_pub, pubkey + key_len, key_len ) ) != 0 )
    {
        DBGLN("READ BINARY");
        pubkey_error += 1;
    }

    // import public key
    if( ( ret = mbedtls_rsa_import( &rsa_pub, &N_pub, NULL, NULL, NULL, &E_pub ) ) != 0 )
    {
        DBGLN("IMPORT");
        pubkey_error += 1;
    }

    // complete public key
    if( ( ret = mbedtls_rsa_complete( &rsa_pub ) ) != 0 )
    {
        DBGLN("COMPLETE");
        pubkey_error += 1;
    }

    // encrypt using public key
    ret = mbedtls_rsa_pkcs1_encrypt( &rsa_pub, mbedtls_ctr_drbg_random,
                                     &ctr_drbg, MBEDTLS_RSA_PUBLIC,
                                     len, input, output );

    if( ret != 0 )
    {
        DBGLN("FAILED ENCRYPT");
    }

    mbedtls_mpi_free( &N_pub ); mbedtls_mpi_free( &E_pub );
    mbedtls_rsa_free( &rsa_pub );

    mbedtls_ctr_drbg_free( &ctr_drbg );
    mbedtls_entropy_free( &entropy );

    return ret;
}

int rsa_decrypt(mbedtls_rsa_context *rsa, mbedtls_ctr_drbg_context *ctr_drbg, const unsigned char *ciphertext, unsigned char *decryptedtext, size_t *olen, size_t max_len)
{
    int ret = 1;

    // decrypt cipertext encrypted using public key
    ret = mbedtls_rsa_pkcs1_decrypt( rsa, mbedtls_ctr_drbg_random,
                                            ctr_drbg, MBEDTLS_RSA_PRIVATE, olen,
                                            ciphertext, decryptedtext, max_len );
    return ret;
}

void setup()
{
    int ret = 1;
    int exit_code = MBEDTLS_EXIT_FAILURE;

    mbedtls_rsa_context rsa;
    mbedtls_ctr_drbg_context ctr_drbg;
    const char *pers = "rsa_genkey";

    rsa_init(&rsa, &ctr_drbg);

    ret = rsa_generate_key(&rsa, &ctr_drbg, pers, strlen(pers));
    if ( ret != 0 )
        DBGLN("FAILED GENERATE KEY");

    ret = rsa_export_pubkey(&rsa, pubkey);
    if ( ret != 0 )
        DBGLN("FAILED EXPORT PUBKEY");

    // encrypt using public key
    unsigned char plaintext_pub[1024] = {0};
    unsigned char ciphertext_pub[512] = {0};
    plaintext_pub[0] = 0x04; plaintext_pub[1] = 0x03; plaintext_pub[2] = 0x02; plaintext_pub[3] = 0x01; plaintext_pub[4] = 0x00;
    ret = rsa_encrypt(pubkey, rsa.len, plaintext_pub, ciphertext_pub, 5);
    if( ret != 0 )
        DBGLN("FAILED ENCRYPT");

    // encrypt using original key
    unsigned char plaintext[1024] = {0};
    unsigned char ciphertext[512] = {0};
    plaintext[0] = 0x00; plaintext[1] = 0x01; plaintext[2] = 0x02; plaintext[3] = 0x03; plaintext[4] = 0x04;
    ret = rsa_encrypt(&rsa, &ctr_drbg, plaintext, ciphertext, 5);
    if( ret != 0 )
        DBGLN("FAILED ENCRYPT");

    unsigned char decryptedtext[1024] = {0};
    size_t i;

    // decrypt using original key
    ret = rsa_decrypt(&rsa, &ctr_drbg, ciphertext, decryptedtext, &i, 1024);
    ret = rsa_decrypt(&rsa, &ctr_drbg, ciphertext_pub, decryptedtext, &i, 1024);
    if( ret != 0 )
        DBGLN("FAILED DECRYPT");

    exit_code = MBEDTLS_EXIT_SUCCESS;

    mbedtls_rsa_free( &rsa );
    mbedtls_ctr_drbg_free( &ctr_drbg );
    mbedtls_exit( exit_code );

  otaPkt.std.type = 0;
  otaPkt.std.crcHigh = 0;
  otaPkt.std.rc.ch.raw[0] = 1;
  otaPkt.std.rc.ch.raw[1] = 2;
  otaPkt.std.rc.ch.raw[2] = 3;
  otaPkt.std.rc.ch.raw[3] = 4;
  otaPkt.std.rc.ch.raw[4] = 5;
  otaPkt.std.rc.switches = 6;
  otaPkt.std.rc.ch4 = 0;
  otaPkt.std.crcLow = 7;


  lea_gcm.init();

  Serial.begin(115200);
  Serial.println("Begin SX1280 testing...");

  Radio.Begin();
  Radio.Config(SX1280_LORA_BW_0800, SX1280_LORA_SF6, SX1280_LORA_CR_LI_4_8, 0xba1b91, 12, true, DATA_SIZE, 20000, 0, 0, 0);
  Radio.TXdoneCallback = &TXdoneCallback;
  Radio.RXdoneCallback = &RXdoneCallback;
  Radio.SetFrequencyHz(2420000000, transmittingRadio);
  //    Radio.RXnb();


  //OTA_Packet_s * const otaPktPtr = (OTA_Packet_s * const)plaintext;

  if (lea_gcm.encrypt(&otaPkt, ciphertext, 32) != 0) {
    DBGLN("LEA GCM encrypt error");
    return;
  }

  Radio.TXnb(ciphertext, sizeof(ciphertext), transmittingRadio);
  //Radio.TXnb(testdata, sizeof(testdata), transmittingRadio);
}

void loop()
{
}
