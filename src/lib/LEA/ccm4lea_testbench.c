// CCM4LEA Testbench
//
// by Joungil Yun (sigipus@etri.re.kr)
//
// 2023.10.

#include "ccm4lea.h"

#define  ARM_CM_DEMCR      (*(uint32_t *)0xE000EDFC)
#define  ARM_CM_DWT_CTRL   (*(uint32_t *)0xE0001000)
#define  ARM_CM_DWT_CYCCNT (*(uint32_t *)0xE0001004)


int ccm4lea_testbench()
{
	CCM_st ccm;

    int i;
    int ret;

    uint32_t  start[5];
	uint32_t  stop[5];
	uint32_t  delta[5];

    if (ARM_CM_DWT_CTRL != 0) 			// See if DWT is available
    {
		ARM_CM_DEMCR      |= 1 << 24;	// Set bit 24
		ARM_CM_DWT_CYCCNT  = 0;
		ARM_CM_DWT_CTRL   |= 1 << 0;	// Set bit 0
    }


// Test Vectors given in Sec. 7.1.1. & 7.1.2. of [KS X 3274] "n비트 블록 암호 운영 모드 — 제2부: 블록 암호 LEA"
#define _TEST_1_ // Select test case in { _TEST_1, _TEST_2_, _TEST_3_, _TEST_4_, _TEST_5_, _TEST_6_ }

#if defined(_TEST_1_)
    uint32_t key_bit_length = 128;
    uint8_t hex_ccm4lea_K[]  = "B56B77B6046901D5B0877C22D054D035";
    uint32_t hex_ccm4lea_K_byte_length = (uint32_t)(sizeof(hex_ccm4lea_K)-1);
    
    uint8_t hex_ccm4lea_N[]  = "AACC9691C820D1844EB6D84E";
    uint32_t hex_ccm4lea_N_byte_length = (uint32_t)(sizeof(hex_ccm4lea_N)-1);

    uint8_t hex_ccm4lea_A[]  = "8A719785CD32F7B7799F9B525C23CDC13C35B8D8B3DED7723E75AD5890AEB765";
    uint32_t hex_ccm4lea_A_byte_length = (uint32_t)(sizeof(hex_ccm4lea_A)-1);

    uint8_t hex_ccm4lea_PP[] = "2CD39FE83250B24A797594F1DF783263DDB9F9AF5F4A4609D2AF1DB47EE21D25169BB3658D";
    uint32_t hex_ccm4lea_PP_byte_length = (uint32_t)(sizeof(hex_ccm4lea_PP)-1);

    uint8_t hex_ccm4lea_CC[] = "8D8E1E4F247E3DC989ED4DA825FD033957619575DE2758FE9AEE24D8638B0060FF5CD75D66";
    uint32_t hex_ccm4lea_CC_byte_length = (uint32_t)(sizeof(hex_ccm4lea_CC)-1);

    uint8_t hex_ccm4lea_T[]  = "59E2C60B10134E367782CF82D4E3E7CF";
    uint32_t hex_ccm4lea_T_byte_length = (uint32_t)(sizeof(hex_ccm4lea_T)-1);

#elif defined(_TEST_2_)
    uint32_t key_bit_length = 192;
    uint8_t hex_ccm4lea_K[]  = "584A897BC5F5F04C855EF3C7C2DC1DA074554A9276035B46";
    uint32_t hex_ccm4lea_K_byte_length = (uint32_t)(sizeof(hex_ccm4lea_K)-1);

    uint8_t hex_ccm4lea_N[]  = "DF782E778FE0CE540887E625";
    uint32_t hex_ccm4lea_N_byte_length = (uint32_t)(sizeof(hex_ccm4lea_N)-1);

    uint8_t hex_ccm4lea_A[]  = "809B7C323432DBE0FE8ED11E2AA01BFF86CC1EA8B8ECD86DD49715372EA3E629";
    uint32_t hex_ccm4lea_A_byte_length = (uint32_t)(sizeof(hex_ccm4lea_A)-1);

    uint8_t hex_ccm4lea_PP[] = "6DE84F83E14740C617ECC619AACF83EB5B815CDA642571D30ABC527B72A770CEB566D10636";
    uint32_t hex_ccm4lea_PP_byte_length = (uint32_t)(sizeof(hex_ccm4lea_PP)-1);

    uint8_t hex_ccm4lea_CC[] = "9AC4FAEC05F61654F0BE3C3F0C1087C65751EAA7D284FA36FF52F20059F98F69E963D5A7AF";
    uint32_t hex_ccm4lea_CC_byte_length = (uint32_t)(sizeof(hex_ccm4lea_CC)-1);

    uint8_t hex_ccm4lea_T[]  = "93E91D503727DA6E57AD40CCCEB161BC";
    uint32_t hex_ccm4lea_T_byte_length = (uint32_t)(sizeof(hex_ccm4lea_T)-1);

#elif defined(_TEST_3_)
    uint32_t key_bit_length = 256;
    uint8_t hex_ccm4lea_K[]  = "EE8DAA519690F3C398BC14AD2F343ADD05C7814649B9AFED02C62206A3640759";
    uint32_t hex_ccm4lea_K_byte_length = (uint32_t)(sizeof(hex_ccm4lea_K)-1);

    uint8_t hex_ccm4lea_N[]  = "DF49CDEBBE2DD9DBF1D17C8A";
    uint32_t hex_ccm4lea_N_byte_length = (uint32_t)(sizeof(hex_ccm4lea_N)-1);

    uint8_t hex_ccm4lea_A[]  = "634022FE1054CC891A7DC0A519BE7E23B3F8F7856E6BFAA8C17AEFA671AEC7FA";
    uint32_t hex_ccm4lea_A_byte_length = (uint32_t)(sizeof(hex_ccm4lea_A)-1);

    uint8_t hex_ccm4lea_PP[] = "1BFE33E50C4E1411593533752D0333785D03C45942FD19CEC7FF9BEBC15DDF467FF7D559A2";
    uint32_t hex_ccm4lea_PP_byte_length = (uint32_t)(sizeof(hex_ccm4lea_PP)-1);

    uint8_t hex_ccm4lea_CC[] = "EE1582B399DE634F74AC11752D8B7DEB711671211A25988F69595501B4DDE9330808B324C6";
    uint32_t hex_ccm4lea_CC_byte_length = (uint32_t)(sizeof(hex_ccm4lea_CC)-1);

    uint8_t hex_ccm4lea_T[]  = "183BAEEFFBD0B80BCED41D3CB5F91D75";
    uint32_t hex_ccm4lea_T_byte_length = (uint32_t)(sizeof(hex_ccm4lea_T)-1);

#elif defined(_TEST_4_)
    uint32_t key_bit_length = 128;
    uint8_t hex_ccm4lea_K[]  = "CD8FB72C74547BE28E9452DB67FB8353";
    uint32_t hex_ccm4lea_K_byte_length = (uint32_t)(sizeof(hex_ccm4lea_K)-1);

    uint8_t hex_ccm4lea_N[]  = "1ACA9DAD55875EBF5CF3D471";
    uint32_t hex_ccm4lea_N_byte_length = (uint32_t)(sizeof(hex_ccm4lea_N)-1);

    uint8_t hex_ccm4lea_A[]  = "513DC286B27545B92343B70C6ED9D45DC8261B3A564D77F6EF551FFC2422BB64";
    uint32_t hex_ccm4lea_A_byte_length = (uint32_t)(sizeof(hex_ccm4lea_A)-1);

    uint8_t hex_ccm4lea_PP[] = "7DA4C8C4064E3D490ACA6F061FEB328B62E341BA04B6E0D606EEBFDD40912218A8503FF0D1";
    uint32_t hex_ccm4lea_PP_byte_length = (uint32_t)(sizeof(hex_ccm4lea_PP)-1);

    uint8_t hex_ccm4lea_CC[] = "CCD6452ACA16136695A23B61B98B43540A90864661859F2B5E972F0C80489580CECB91D366";
    uint32_t hex_ccm4lea_CC_byte_length = (uint32_t)(sizeof(hex_ccm4lea_CC)-1);

    uint8_t hex_ccm4lea_T[]  = "7F6D61A0DAA894C77CADC4E30F9017A5";
    uint32_t hex_ccm4lea_T_byte_length = (uint32_t)(sizeof(hex_ccm4lea_T)-1);

#elif defined(_TEST_5_)
    uint32_t key_bit_length = 192;
    uint8_t hex_ccm4lea_K[]  = "A608BF347B65589BEA699C20AA7CA28C2BE2F158D8ECE87B";
    uint32_t hex_ccm4lea_K_byte_length = (uint32_t)(sizeof(hex_ccm4lea_K)-1);

    uint8_t hex_ccm4lea_N[]  = "9A6B5E809D6A85DB561D147D";
    uint32_t hex_ccm4lea_N_byte_length = (uint32_t)(sizeof(hex_ccm4lea_N)-1);

    uint8_t hex_ccm4lea_A[]  = "C514A5A6F7B45BFBD0E8C9D807140FE7724B46F284C11FF51483C6785F650D1B";
    uint32_t hex_ccm4lea_A_byte_length = (uint32_t)(sizeof(hex_ccm4lea_A)-1);

    uint8_t hex_ccm4lea_PP[] = "513CC673004E70147A0717EFC0EE75DF12D75D7E6D595A3D538293B4CACAF136A74684A90C";
    uint32_t hex_ccm4lea_PP_byte_length = (uint32_t)(sizeof(hex_ccm4lea_PP)-1);

    uint8_t hex_ccm4lea_CC[] = "061629D478E080AA0CCB681C9418DB9529DE05DA0B5D7672BC06BFEA6ACAFE23F08416D1AF";
    uint32_t hex_ccm4lea_CC_byte_length = (uint32_t)(sizeof(hex_ccm4lea_CC)-1);

    uint8_t hex_ccm4lea_T[]  = "DD1B19AD7EB6B0C55544A96DCA31888F";
    uint32_t hex_ccm4lea_T_byte_length = (uint32_t)(sizeof(hex_ccm4lea_T)-1);

#elif defined(_TEST_6_)
    uint32_t key_bit_length = 256;
    uint8_t hex_ccm4lea_K[]  = "BA43272F6A7223E39E284DC70CD59E040611A6F745467488E6AAB6B8B271217A";
    uint32_t hex_ccm4lea_K_byte_length = (uint32_t)(sizeof(hex_ccm4lea_K)-1);

    uint8_t hex_ccm4lea_N[]  = "A24013C4A2E477E59644EA19";
    uint32_t hex_ccm4lea_N_byte_length = (uint32_t)(sizeof(hex_ccm4lea_N)-1);

    uint8_t hex_ccm4lea_A[]  = "9B941B5B8FCC787AD359076F4F1291EEB76D063B0DF31D3F68F13BDBF371967C";
    uint32_t hex_ccm4lea_A_byte_length = (uint32_t)(sizeof(hex_ccm4lea_A)-1);

    uint8_t hex_ccm4lea_PP[] = "38689AF09F318C78313A6D1EA2FE21488F33D08E04448DC94101356CBE969A186CB9E16A3E";
    uint32_t hex_ccm4lea_PP_byte_length = (uint32_t)(sizeof(hex_ccm4lea_PP)-1);

    uint8_t hex_ccm4lea_CC[] = "A6E63A51FDFF3F13D08513710E1EE3ACCFF1F014E036C4DA3C803E83BFB1428B4503DE265E";
    uint32_t hex_ccm4lea_CC_byte_length = (uint32_t)(sizeof(hex_ccm4lea_CC)-1);

    uint8_t hex_ccm4lea_T[]  = "6D7FD52BC70CEF6D4E080CD112692251";
    uint32_t hex_ccm4lea_T_byte_length = (uint32_t)(sizeof(hex_ccm4lea_T)-1);

#endif


    uint8_t K[64] = {0, };
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

    K_byte_length 	= hexs2bytes(K, hex_ccm4lea_K, hex_ccm4lea_K_byte_length);
    N_byte_length 	= hexs2bytes(N, hex_ccm4lea_N, hex_ccm4lea_N_byte_length);
    A_byte_length 	= hexs2bytes(A, hex_ccm4lea_A, hex_ccm4lea_A_byte_length);
    PP_byte_length 	= hexs2bytes(PP, hex_ccm4lea_PP, hex_ccm4lea_PP_byte_length);
    CC_byte_length 	= hexs2bytes(CC, hex_ccm4lea_CC, hex_ccm4lea_CC_byte_length);
    T_byte_length 	= hexs2bytes(T, hex_ccm4lea_T, hex_ccm4lea_T_byte_length);

    if((K_byte_length << 3) != key_bit_length)
    {
    	CONSOL_PRINTF("\nERROR! --> Please check the key length");
		return -1;
    }


    // Initializing (w/ LEA Round Key Schedule)
    start[0] = ARM_CM_DWT_CYCCNT;
    ret = CCM4LEA_set_init_params(&ccm, K, key_bit_length, A, A_byte_length, T_byte_length);
    stop[0] = ARM_CM_DWT_CYCCNT;

    if(ret < 0)
    {
    	CONSOL_PRINTF("\nERROR! --> Encryption test failed. @CCM4LEA_set_init_params(%d)\n", ret);
    	return -1;
	}

    ret = CCM4LEA_set_enc_params(&ccm, PP, PP_byte_length, N, N_byte_length);
    if(ret < 0)
	{
		CONSOL_PRINTF("\nERROR! --> Encryption test failed. @CCM4LEA_set_enc_params(%d)\n", ret);
    	return -1;
	}


    // Encryption
    start[1] = ARM_CM_DWT_CYCCNT;
    ret = CCM4LEA_enc(&ccm);
    stop[1] = ARM_CM_DWT_CYCCNT;

    if(ret < 0)
    {
		CONSOL_PRINTF("\nERROR! --> Encryption test failed. @CCM4LEA_enc(%d)\n", ret);
    	return -1;
	}

    if(ccm.CC_byte_length != ccm.PP_byte_length)
	{
		CONSOL_PRINTF("[Input plain text]: ");
		for (i = 0; i < ccm.PP_byte_length; i++)
		{
			if (i % 4 == 0)
			{
				CONSOL_PRINTF(" ");
				if (i % 16 == 0) CONSOL_PRINTF("\n");
			}
			CONSOL_PRINTF("%02x", ccm.PP[i]);
		}
		CONSOL_PRINTF("\n\n");

		CONSOL_PRINTF("[Encrypted text output]: ");
		for (i = 0; i < ccm.CC_byte_length; i++)
		{
				if (i % 4 == 0)
				{
					CONSOL_PRINTF(" ");
					if (i % 16 == 0) CONSOL_PRINTF("\n");
				}
				CONSOL_PRINTF("%02x", ccm.CC[i]);
		}
		CONSOL_PRINTF("\n");

		CONSOL_PRINTF("\nERROR! --> Encryption test failed. (ccm.CC_byte_length != ccm.PP_byte_length)\n");
		return -1;
	}

    if(memcmp(CC, ccm.CC, ccm.CC_byte_length) != 0)
    {
    	CONSOL_PRINTF("[Given text encrypted]: ");
		for (i = 0; i < ccm.CC_byte_length; i++)
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
		for (i = 0; i < ccm.CC_byte_length; i++)
		{
				if (i % 4 == 0)
				{
					CONSOL_PRINTF(" ");
					if (i % 16 == 0) CONSOL_PRINTF("\n");
				}
				CONSOL_PRINTF("%02x", ccm.CC[i]);
		}
		CONSOL_PRINTF("\n\n");


		CONSOL_PRINTF("\nERROR! --> Encryption test failed. (Mismatch of encrypted text data)\n");
		return -1;
    }

    if(memcmp(T, ccm.T, ccm.T_byte_length) != 0)
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
			CONSOL_PRINTF("0x%02x ", ccm.T[i]);
		}
		CONSOL_PRINTF("\n");

		CONSOL_PRINTF("\nERROR! --> Encryption test failed. (Mismatch of T value)\n");
		return -1;
    }

    CONSOL_PRINTF("\n<<<<<<<<<<<<<<<  CCM4LEA Encryption test passed!!!  >>>>>>>>>>>>>>>\n");


    // Initialize again for Decryption Test!
    ret = CCM4LEA_set_init_params(&ccm, K, key_bit_length, A, A_byte_length, T_byte_length);
	if(ret < 0)
	{
		CONSOL_PRINTF("\nERROR! --> Decryption test failed. @CCM4LEA_set_init_params(%d)\n", ret);
		return -1;
	}

	ret = CCM4LEA_set_dec_params(&ccm, CC, CC_byte_length, N, N_byte_length, T);
	if(ret < 0)
	{
		CONSOL_PRINTF("\nERROR! --> Decryption test failed. @CCM4LEA_set_dec_params(%d)\n", ret);
		return -1;
	}


    // Decryption
    start[2] = ARM_CM_DWT_CYCCNT;
    ret = CCM4LEA_dec(&ccm);
    stop[2] = ARM_CM_DWT_CYCCNT;

    if(ret < 0)
    {
		CONSOL_PRINTF("[Generated T]: ");
		for(i = 0; i < 16; i++)
		{
			CONSOL_PRINTF("0x%02x ", ccm.T[i]);
		}
		CONSOL_PRINTF("\n");

		CONSOL_PRINTF("[Given T]: ");
		for(i = 0; i < 16; i++)
		{
			CONSOL_PRINTF("0x%02x ", T[i]);
		}
		CONSOL_PRINTF("\n\n");

		CONSOL_PRINTF("\nERROR! --> Decryption test failed. @CCM4LEA_dec (%d) - Mismatch of T value\n", ret);
		return -1;
    }

    if(ccm.PP_byte_length != ccm.CC_byte_length)
    {
    	CONSOL_PRINTF("[Input Encrypted text]: ");
		for (i = 0; i < ccm.CC_byte_length; i++)
		{
				if (i % 4 == 0)
				{
					CONSOL_PRINTF(" ");
					if (i % 16 == 0) CONSOL_PRINTF("\n");
				}
				CONSOL_PRINTF("%02x", ccm.CC[i]);
		}
		CONSOL_PRINTF("\n\n");

    	CONSOL_PRINTF("[Decrypted text output]: ");
		for (i = 0; i < ccm.PP_byte_length; i++)
		{
				if (i % 4 == 0)
				{
					CONSOL_PRINTF(" ");
					if (i % 16 == 0) CONSOL_PRINTF("\n");
				}
				CONSOL_PRINTF("%02x", ccm.PP[i]);
		}
		CONSOL_PRINTF("\n");

    	CONSOL_PRINTF("\nERROR! --> Decryption test failed. (ccm.PP_byte_length != ccm.CC_byte_length)\n");
        return -1;
    }

    if(memcmp(PP, ccm.PP, ccm.PP_byte_length) != 0)
    {
    	CONSOL_PRINTF("[Given plain text]: ");
		for (i = 0; i < ccm.PP_byte_length; i++)
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
		for (i = 0; i < ccm.PP_byte_length; i++)
		{
				if (i % 4 == 0)
				{
					CONSOL_PRINTF(" ");
					if (i % 16 == 0) CONSOL_PRINTF("\n");
				}
				CONSOL_PRINTF("%02x", ccm.PP[i]);
		}
		CONSOL_PRINTF("\n\n");

    	CONSOL_PRINTF("\nERROR! --> Decryption test failed. (Mismatch of decrypted text data)\n");
        return -1;
    }

	delta[0] = stop[0] - start[0];
	delta[1] = stop[1] - start[1];
	delta[2] = stop[2] - start[2];


    CONSOL_PRINTF("\n>>>>>>>>>>>>>>>  CCM4LEA Decryption test passed!!!  <<<<<<<<<<<<<<<\n\n");

    CONSOL_PRINTF("---------------------------------------------------------------\n");

    CONSOL_PRINTF("  Length of Params:\n");
    CONSOL_PRINTF("    - Key          = %d (bits)\n", (int)key_bit_length);
    CONSOL_PRINTF("    - P, C         = %d (bytes)\n", (int)PP_byte_length);
    CONSOL_PRINTF("    - N            = %d (bytes)\n", (int)N_byte_length);
    CONSOL_PRINTF("    - A            = %d (bytes)\n", (int)A_byte_length);
    CONSOL_PRINTF("    - T            = %d (bytes)\n", (int)T_byte_length);

    CONSOL_PRINTF("\n  Elapsed Cycles:\n");
    CONSOL_PRINTF("    - Initializing = %d (cycles)\n", (int)delta[0]);
    CONSOL_PRINTF("    - Encryption   = %d (cycles), %f (cycle/byte)\n", (int)delta[1], ((float)delta[1] / (float)PP_byte_length));
    CONSOL_PRINTF("    - Decryption   = %d (cycles), %f (cycle/byte)\n", (int)delta[2], ((float)delta[2] / (float)PP_byte_length));

    CONSOL_PRINTF("---------------------------------------------------------------\n");

    return 0;
}
// End of ccm4lea_testbench.c
