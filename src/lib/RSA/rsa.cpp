#include "rsa.h"

RSA::RSA() {
    mbedtls_entropy_init( &entropy_ );
    mbedtls_ctr_drbg_init( &ctr_drbg_);
    mbedtls_rsa_init( &rsa_, MBEDTLS_RSA_PKCS_V15, 0);
}

RSA::~RSA() {
    mbedtls_rsa_free(&rsa_);
    mbedtls_ctr_drbg_free(&ctr_drbg_);
    mbedtls_entropy_free( &entropy_ );
}

int RSA::generate_key(const char *pers, size_t pers_len)
{
    int ret = 1;
    int exit_code = MBEDTLS_EXIT_FAILURE;

    if( ( ret = mbedtls_ctr_drbg_seed( &ctr_drbg_, mbedtls_entropy_func, &entropy_,
                               (const unsigned char *) pers_, strlen(pers_) ) ) != 0 )
    {
        // TODO: -52 return error handling
        // __BKPT();
        //Serial.println( " failed\n  ! mbedtls_ctr_drbg_seed returned %d\n", ret );
        DBGLN("SEED");
    }

    if( ( ret = mbedtls_rsa_gen_key( &rsa_, mbedtls_ctr_drbg_random, &ctr_drbg_, KEY_SIZE,
                                     EXPONENT ) ) != 0 )
    {
        //Serial.println( " failed\n  ! mbedtls_rsa_gen_key returned %d\n\n", ret );
        DBGLN("GEN KEY");
    }

    return ret;
}

int RSA::export_pubkey(unsigned char *pubkey, size_t *pubkey_len)
{
    int ret = 1;
    int exit_code = MBEDTLS_EXIT_FAILURE;

    mbedtls_mpi N, E;
    mbedtls_mpi_init( &N ); mbedtls_mpi_init( &E );

    if( ( ret = mbedtls_rsa_export( &rsa_, &N, NULL, NULL, NULL, &E ) ) != 0 )
    {
        //mbedtls_printf( " failed\n  ! could not export RSA parameters\n\n" );
        DBGLN("EXPORT");
        return ret;
    }

    // save public key
    mbedtls_mpi_write_binary( &N, pubkey, rsa_.len );
    mbedtls_mpi_write_binary( &E, pubkey + rsa_.len, rsa_.len );
    *pubkey_len = rsa_.len * 2;

    mbedtls_mpi_free( &N ); mbedtls_mpi_free( &E );

    return ret;
}


int RSA::encrypt(const unsigned char *input, unsigned char *output, size_t len)
{
    int ret = 1;
    int exit_code = MBEDTLS_EXIT_FAILURE;

    ret = mbedtls_rsa_pkcs1_encrypt( &rsa_, mbedtls_ctr_drbg_random,
                                     &ctr_drbg_, MBEDTLS_RSA_PUBLIC,
                                     len, input, output );

    if( ret != 0 )
    {
        DBGLN("FAILED ENCRYPT");
    }

    return ret;
}

int RSA::encrypt(const unsigned char *pubkey, size_t pubkey_len, const unsigned char *input, unsigned char *output, size_t len)
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
    int pubkey_ne_len = pubkey_len / 2;
    if( ( ret = mbedtls_mpi_read_binary( &N_pub, pubkey, pubkey_ne_len ) ) != 0  ||
        ( ret = mbedtls_mpi_read_binary( &E_pub, pubkey + pubkey_ne_len, pubkey_ne_len ) ) != 0 )
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


int RSA::decrypt(const unsigned char *ciphertext, unsigned char *decryptedtext, size_t *olen, size_t max_len)
{
    int ret = 1;

    // decrypt cipertext encrypted using public key
    ret = mbedtls_rsa_pkcs1_decrypt( &rsa_, mbedtls_ctr_drbg_random,
                                            &ctr_drbg_, MBEDTLS_RSA_PRIVATE, olen,
                                            ciphertext, decryptedtext, max_len );
    return ret;
}
