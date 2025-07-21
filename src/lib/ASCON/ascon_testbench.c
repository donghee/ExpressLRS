// ASCON Testbench
//
// by Joungil Yun (sigipus@etri.re.kr)
//
// 2024.07.

#include "ascon128x.h"

#define  ARM_CM_DEMCR      (*(volatile uint32_t *)0xE000EDFC)
#define  ARM_CM_DWT_CTRL   (*(volatile uint32_t *)0xE0001000)
#define  ARM_CM_DWT_CYCCNT (*(volatile uint32_t *)0xE0001004)
#define  ARM_CM_DWT_LAR	   (*(volatile uint32_t *)0xE0001FB0)

#define _TIMES_ 100

int ascon_testbench()
{
	ASCON_st ascon;

    int i;
    int ret;

    uint32_t  start[5];
	uint32_t  stop[5];
	uint32_t  delta[5];

    if (ARM_CM_DWT_CTRL != 0) 				// See if DWT is available
    {
    	ARM_CM_DWT_LAR     = 0xC5ACCE55;	// Unlock access to DWT (ITM, etc.) registers
		ARM_CM_DEMCR      |= 0x01000000;	// Enable trace. Set bit 24 (1 << 24)
		ARM_CM_DWT_CYCCNT  = 0;				// Clear DWT cycle counter.
		ARM_CM_DWT_CTRL   |= 0;				// Enable DWT cycle counter. Set bit 0 (1 << 0)
    }

#if (ASCON_AEAD_RATE == 8)
    uint32_t key_bit_length = 128;
    uint8_t hex_ascon_K[]  = "000102030405060708090A0B0C0D0E0F";
    uint32_t hex_ascon_K_byte_length = (uint32_t)(sizeof(hex_ascon_K)-1);

    uint8_t hex_ascon_N[]  = "000102030405060708090A0B0C0D0E0F";
    uint32_t hex_ascon_N_byte_length = (uint32_t)(sizeof(hex_ascon_N)-1);

    uint8_t hex_ascon_A[]  = "000102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F202122232425262728292A2B2C2D2E2F303132333435363738393A3B3C3D3E3F404142434445464748494A4B4C4D4E4F505152535455565758595A5B5C5D5E5F606162636465666768696A6B6C6D6E6F707172737475767778797A7B7C7D7E7F";
    uint32_t hex_ascon_A_byte_length = (uint32_t)(sizeof(hex_ascon_A)-1);

    uint8_t hex_ascon_PP[] = "000102030405060708090A0B0C0D0E0F";
    uint32_t hex_ascon_PP_byte_length = (uint32_t)(sizeof(hex_ascon_PP)-1);

    uint8_t hex_ascon_CC[] = "5A08D8DF97CF5CA04FA4313B1A6B41BE";
    uint32_t hex_ascon_CC_byte_length = (uint32_t)(sizeof(hex_ascon_CC)-1);

    uint8_t hex_ascon_T[]  = "BB432AAD3BD1D5DF77F2B6E9A0CAE107";
    uint32_t hex_ascon_T_byte_length = (uint32_t)(sizeof(hex_ascon_T)-1);
#elif (ASCON_AEAD_RATE == 16)
    uint32_t key_bit_length = 128;
    uint8_t hex_ascon_K[]  = "000102030405060708090A0B0C0D0E0F";
    uint32_t hex_ascon_K_byte_length = (uint32_t)(sizeof(hex_ascon_K)-1);

    uint8_t hex_ascon_N[]  = "000102030405060708090A0B0C0D0E0F";
    uint32_t hex_ascon_N_byte_length = (uint32_t)(sizeof(hex_ascon_N)-1);

    uint8_t hex_ascon_A[]  = "000102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F202122232425262728292A2B2C2D2E2F303132333435363738393A3B3C3D3E3F404142434445464748494A4B4C4D4E4F505152535455565758595A5B5C5D5E5F606162636465666768696A6B6C6D6E6F707172737475767778797A7B7C7D7E7F";
    uint32_t hex_ascon_A_byte_length = (uint32_t)(sizeof(hex_ascon_A)-1);

    uint8_t hex_ascon_PP[] = "000102030405060708090A0B0C0D0E0F";
    uint32_t hex_ascon_PP_byte_length = (uint32_t)(sizeof(hex_ascon_PP)-1);

    uint8_t hex_ascon_CC[] = "9DDB43D500FC53FB8C9C8E319E57F8C2";
    uint32_t hex_ascon_CC_byte_length = (uint32_t)(sizeof(hex_ascon_CC)-1);

    uint8_t hex_ascon_T[]  = "6ED7181A0E07412C65BFA12A69224CF9";
    uint32_t hex_ascon_T_byte_length = (uint32_t)(sizeof(hex_ascon_T)-1);
#endif


    uint8_t K[16] = {0, };
    uint8_t PP[MAX_P_C_BYTE_LENGTH] = {0, };
    uint8_t CC[MAX_P_C_BYTE_LENGTH] = {0, };
    uint8_t N[16] = {0, };
    uint8_t A[MAX_A_BYTE_LENGTH] = {0, };
    uint8_t T[16] = {0, };

    uint32_t K_byte_length;
    uint32_t PP_byte_length;
    uint32_t CC_byte_length;
    uint32_t N_byte_length;
    uint32_t A_byte_length;
    uint32_t T_byte_length;


    K_byte_length 	= hexs2bytes(K, hex_ascon_K, hex_ascon_K_byte_length);
    N_byte_length 	= hexs2bytes(N, hex_ascon_N, hex_ascon_N_byte_length);
    A_byte_length 	= hexs2bytes(A, hex_ascon_A, hex_ascon_A_byte_length);
    PP_byte_length 	= hexs2bytes(PP, hex_ascon_PP, hex_ascon_PP_byte_length);
    CC_byte_length 	= hexs2bytes(CC, hex_ascon_CC, hex_ascon_CC_byte_length);
    T_byte_length 	= hexs2bytes(T, hex_ascon_T, hex_ascon_T_byte_length);


    if((K_byte_length << 3) != key_bit_length)
	{
		CONSOL_PRINTF("\nERROR! --> Please check the key length  %d", K_byte_length);
		return -1;
	}

    if(N_byte_length != 16)
    {
    	CONSOL_PRINTF("\nERROR! --> Please check the nonce length");
    	return -1;
    }


    // Initializing
    delta[0] = 0;
    for(i = 0; i < _TIMES_; i++)
    {
		start[0] = ARM_CM_DWT_CYCCNT;
		ret = ASCON128x_set_init_params(&ascon, K, key_bit_length, A, A_byte_length, T_byte_length);
		stop[0] = ARM_CM_DWT_CYCCNT;
		delta[0] += (stop[0] - start[0]) - 2;
    }

    if(ret < 0)
    {
    	CONSOL_PRINTF("\nERROR! --> Encryption test failed. @ASCON128x_set_init_params(%d)\n", ret);
    	return -1;
	}

    ret = ASCON128x_set_enc_params(&ascon, PP, PP_byte_length, N, N_byte_length);
    if(ret < 0)
	{
		CONSOL_PRINTF("\nERROR! --> Encryption test failed. @ASCON128x_set_enc_params(%d)\n", ret);
    	return -1;
	}


    // Encryption
    delta[1] = 0;
	for(i = 0; i < _TIMES_; i++)
	{
		start[1] = ARM_CM_DWT_CYCCNT;
		ret = ASCON128x_enc(&ascon);
		stop[1] = ARM_CM_DWT_CYCCNT;
		delta[1] += (stop[1] - start[1]) - 2;
    }

    if(ret < 0)
    {
		CONSOL_PRINTF("\nERROR! --> Encryption test failed. @ASCON128x_enc(%d)\n", ret);
    	return -1;
	}

    if(ascon.CC_byte_length != ascon.PP_byte_length)
	{
		CONSOL_PRINTF("[Input plain text]: ");
		for (i = 0; i < ascon.PP_byte_length; i++)
		{
			if (i % 4 == 0)
			{
				CONSOL_PRINTF(" ");
				if (i % 16 == 0) CONSOL_PRINTF("\n");
			}
			CONSOL_PRINTF("%02x", ascon.PP[i]);
		}
		CONSOL_PRINTF("\n\n");

		CONSOL_PRINTF("[Encrypted text output]: ");
		for (i = 0; i < ascon.CC_byte_length; i++)
		{
				if (i % 4 == 0)
				{
					CONSOL_PRINTF(" ");
					if (i % 16 == 0) CONSOL_PRINTF("\n");
				}
				CONSOL_PRINTF("%02x", ascon.CC[i]);
		}
		CONSOL_PRINTF("\n");

		CONSOL_PRINTF("\nERROR! --> Encryption test failed. (ascon.CC_byte_length != ascon.PP_byte_length)\n");
		return -1;
	}

    if(memcmp(CC, ascon.CC, ascon.CC_byte_length) != 0)
    {
    	CONSOL_PRINTF("[Given text encrypted]: ");
		for (i = 0; i < ascon.CC_byte_length; i++)
		{
				if (i % 4 == 0)
				{
					CONSOL_PRINTF(" ");
					if (i % 16 == 0) CONSOL_PRINTF("\n");
				}
				CONSOL_PRINTF("%02x", CC[i]);
		}
		CONSOL_PRINTF("\n");

		CONSOL_PRINTF("[Encrypted text output]: ");
		for (i = 0; i < ascon.CC_byte_length; i++)
		{
				if (i % 4 == 0)
				{
					CONSOL_PRINTF(" ");
					if (i % 16 == 0) CONSOL_PRINTF("\n");
				}
				CONSOL_PRINTF("%02x", ascon.CC[i]);
		}
		CONSOL_PRINTF("\n\n");


		CONSOL_PRINTF("\nERROR! --> Encryption test failed. (Mismatch of encrypted text data)\n");
		return -1;
    }

    if(memcmp(T, ascon.T, ascon.T_byte_length) != 0)
    {
    	CONSOL_PRINTF("[Given T]: ");
		for(i = 0; i < 16; i++)
		{
			CONSOL_PRINTF("0x%02x ", T[i]);
		}
		CONSOL_PRINTF("\n\n");

    	CONSOL_PRINTF("[output T]: ");
		for(i = 0; i < 16; i++)
		{
			CONSOL_PRINTF("0x%02x ", ascon.T[i]);
		}
		CONSOL_PRINTF("\n");

		CONSOL_PRINTF("\nERROR! --> Encryption test failed. (Mismatch of T value)\n");
		return -1;
    }

    CONSOL_PRINTF("\n<<<<<<<<<<<<<<<  ASCON128x Encryption test passed!!!  >>>>>>>>>>>>>>>\n");


    // Initialize again for Decryption Test!
    ret = ASCON128x_set_init_params(&ascon, K, key_bit_length, A, A_byte_length, T_byte_length);
	if(ret < 0)
	{
		CONSOL_PRINTF("\nERROR! --> Decryption test failed. @ASCON128x_set_init_params(%d)\n", ret);
		return -1;
	}

	ret = ASCON128x_set_dec_params(&ascon, CC, CC_byte_length, N, N_byte_length, T);
	if(ret < 0)
	{
		CONSOL_PRINTF("\nERROR! --> Decryption test failed. @ASCON128x_set_dec_params(%d)\n", ret);
		return -1;
	}


    // Decryption
    delta[2] = 0;
	for(i = 0; i < _TIMES_; i++)
	{
		start[2] = ARM_CM_DWT_CYCCNT;
		ret = ASCON128x_dec(&ascon);
		stop[2] = ARM_CM_DWT_CYCCNT;
		delta[2] += (stop[2] - start[2]) - 2;
	}

    if(ret < 0)
    {
		CONSOL_PRINTF("[Generated T]: ");
		for(i = 0; i < 16; i++)
		{
			CONSOL_PRINTF("0x%02x ", ascon.T[i]);
		}
		CONSOL_PRINTF("\n");

		CONSOL_PRINTF("[Given T]: ");
		for(i = 0; i < 16; i++)
		{
			CONSOL_PRINTF("0x%02x ", T[i]);
		}
		CONSOL_PRINTF("\n\n");

		CONSOL_PRINTF("\nERROR! --> Decryption test failed. @ASCON128x_dec (%d) - Mismatch of T value\n", ret);
		return -1;
    }

    if(ascon.PP_byte_length != ascon.CC_byte_length)
    {
    	CONSOL_PRINTF("[Input Encrypted text]: ");
		for (i = 0; i < ascon.CC_byte_length; i++)
		{
				if (i % 4 == 0)
				{
					CONSOL_PRINTF(" ");
					if (i % 16 == 0) CONSOL_PRINTF("\n");
				}
				CONSOL_PRINTF("%02x", ascon.CC[i]);
		}
		CONSOL_PRINTF("\n\n");

    	CONSOL_PRINTF("[Decrypted text output]: ");
		for (i = 0; i < ascon.PP_byte_length; i++)
		{
				if (i % 4 == 0)
				{
					CONSOL_PRINTF(" ");
					if (i % 16 == 0) CONSOL_PRINTF("\n");
				}
				CONSOL_PRINTF("%02x", ascon.PP[i]);
		}
		CONSOL_PRINTF("\n");

    	CONSOL_PRINTF("\nERROR! --> Decryption test failed. (ascon.PP_byte_length != ascon.CC_byte_length)\n");
        return -1;
    }

    if(memcmp(PP, ascon.PP, ascon.PP_byte_length) != 0)
    {
    	CONSOL_PRINTF("[Given plain text]: ");
		for (i = 0; i < ascon.PP_byte_length; i++)
		{
				if (i % 4 == 0)
				{
					CONSOL_PRINTF(" ");
					if (i % 16 == 0) CONSOL_PRINTF("\n");
				}
				CONSOL_PRINTF("%02x", PP[i]);
		}
		CONSOL_PRINTF("\n");

		CONSOL_PRINTF("[Decrypted text output]: ");
		for (i = 0; i < ascon.PP_byte_length; i++)
		{
				if (i % 4 == 0)
				{
					CONSOL_PRINTF(" ");
					if (i % 16 == 0) CONSOL_PRINTF("\n");
				}
				CONSOL_PRINTF("%02x", ascon.PP[i]);
		}
		CONSOL_PRINTF("\n\n");

    	CONSOL_PRINTF("\nERROR! --> Decryption test failed. (Mismatch of decrypted text data)\n");
        return -1;
    }

	//delta[0] = stop[0] - start[0];
	//delta[1] = stop[1] - start[1];
	//delta[2] = stop[2] - start[2];


    CONSOL_PRINTF("\n>>>>>>>>>>>>>>>  ASCON128x Decryption test passed!!!  <<<<<<<<<<<<<<<\n\n");

    CONSOL_PRINTF("---------------------------------------------------------------\n");

    CONSOL_PRINTF("  ASCON Algorithm Info:\n");

    CONSOL_PRINTF("    - rate         = %d (bytes)\n", (int)ASCON_AEAD_RATE);
#if (ASCON_AEAD_RATE == 8)
    CONSOL_PRINTF("    : ASCON-128\n");
#elif (ASCON_AEAD_RATE == 16)
    CONSOL_PRINTF("    : ASCON-128a\n");
#endif

    CONSOL_PRINTF("\n  Length of Params:\n");
    CONSOL_PRINTF("    - Key          = %d (bits)\n", (int)key_bit_length);
    CONSOL_PRINTF("    - P, C         = %d (bytes)\n", (int)PP_byte_length);
    CONSOL_PRINTF("    - N            = %d (bytes)\n", (int)N_byte_length);
    CONSOL_PRINTF("    - A            = %d (bytes)\n", (int)A_byte_length);
    CONSOL_PRINTF("    - T            = %d (bytes)\n", (int)T_byte_length);

    CONSOL_PRINTF("\n  Elapsed Cycles: (average cycle for %d iterations)\n", _TIMES_);
    CONSOL_PRINTF("    - Initializing = %f (cycles)\n", ((float)delta[0] / _TIMES_));
    CONSOL_PRINTF("    - Encryption   = %f (cycles), %f (cycle/byte)\n", ((float)delta[1] / _TIMES_), ((float)delta[1] / (float)PP_byte_length) / _TIMES_);
    CONSOL_PRINTF("    - Decryption   = %f (cycles), %f (cycle/byte)\n", ((float)delta[2] / _TIMES_), ((float)delta[2] / (float)PP_byte_length) / _TIMES_);

    CONSOL_PRINTF("---------------------------------------------------------------\n");



    return 0;
}
// End of ascon_testbench.c
