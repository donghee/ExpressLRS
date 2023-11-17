// GCM4LEA Testbench
//
// by Joungil Yun (sigipus@etri.re.kr)
//
// 2023.10.

#include "gcm4lea.h"

#define  ARM_CM_DEMCR      (*(uint32_t *)0xE000EDFC)
#define  ARM_CM_DWT_CTRL   (*(uint32_t *)0xE0001000)
#define  ARM_CM_DWT_CYCCNT (*(uint32_t *)0xE0001004)


int gcm4lea_testbench()
{
	GCM_st gcm;

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


// Test Vectors given in Sec. 7.2.1. & 7.2.2. of [KS X 3274] "n비트 블록 암호 운영 모드 — 제2부: 블록 암호 LEA"
#define _TEST_1_ // Select test case in { _TEST_1, _TEST_2_, _TEST_3_, _TEST_4_, _TEST_5_, _TEST_6_ }

#if defined(_TEST_1_)
    uint32_t key_bit_length = 128;
    uint8_t hex_gcm4lea_K[]  = "14870B9992EA89678A1DDFD630918DF0";
    uint32_t hex_gcm4lea_K_byte_length = (uint32_t)(sizeof(hex_gcm4lea_K)-1);
    
    uint8_t hex_gcm4lea_N[]  = "B5C9D87823B1CCD88830825B56009F9C";
    uint32_t hex_gcm4lea_N_byte_length = (uint32_t)(sizeof(hex_gcm4lea_N)-1);

    uint8_t hex_gcm4lea_A[]  = "CC5AA1A407CE141C6790679BC429CD178E58DC59B36194F8C3A79D88537DD00F9F3AF9B8738BDBCC4B0E75DDB91C844507E649CF1DC38CB3A6C8E1C86F9A2AF50F805DE8CDBC880BBE1C12B42ECC439D416E0693DA6CEE72BC8D7850F0C790F96655569CDF0921C309E2C488ECE1110686199C0FB206C21D4F1FE98A9DAE0803";
    uint32_t hex_gcm4lea_A_byte_length = (uint32_t)(sizeof(hex_gcm4lea_A)-1);

    uint8_t hex_gcm4lea_PP[] = "0ABE1D3F0C3D4EDED6201D4659A898E4";
    uint32_t hex_gcm4lea_PP_byte_length = (uint32_t)(sizeof(hex_gcm4lea_PP)-1);

    uint8_t hex_gcm4lea_CC[] = "7E25BE7113562661E660AB884BDDD8CE";
    uint32_t hex_gcm4lea_CC_byte_length = (uint32_t)(sizeof(hex_gcm4lea_CC)-1);

    uint8_t hex_gcm4lea_T[]  = "98c6fc05a69ede0ee404a6ec9121ffda";
    uint32_t hex_gcm4lea_T_byte_length = (uint32_t)(sizeof(hex_gcm4lea_T)-1);

#elif defined(_TEST_2_)
    uint32_t key_bit_length = 192;
    uint8_t hex_gcm4lea_K[]  = "1B0E30425534804E8008438C7A416406B3DFF45BE10E7DC0";
    uint32_t hex_gcm4lea_K_byte_length = (uint32_t)(sizeof(hex_gcm4lea_K)-1);

    uint8_t hex_gcm4lea_N[]  = "F54C93454DCEAD5A2B203391";
    uint32_t hex_gcm4lea_N_byte_length = (uint32_t)(sizeof(hex_gcm4lea_N)-1);

    uint8_t hex_gcm4lea_A[]  = "1F8A63FF647B9692658687C5FD6E00076CC165FF4DFF45D0C9FE2F9C0B8DC747F358187D0F0C2CF6C7BBDAF8D781E01E0368325905899134CB745E5CDFD2B15F2749A6B4CEA0F7FC8224D087E04ADE1A2C95AAEF46BA25FED903837BD6F14DA02125B2AC8A801F2CFE8A0F79FE102382511275CBF6DC2AB65D724602D731C491";
    uint32_t hex_gcm4lea_A_byte_length = (uint32_t)(sizeof(hex_gcm4lea_A)-1);

    uint8_t hex_gcm4lea_PP[] = "660DD8A55FBE2ABC372667E93F9D93F5B0CAC75F9DC7CCA12580E9B5ED5CF8D8CE20BF8ECA107EE64557C5FF71A888CB0B267513458185BB0362D248490AA89FF436DE0FA94567920C9C2450324AFC6558E82FE4D5C3E71C4D3E111A28716A6F4216DD33E4A60C49F4BF2DE568C9174C00D99CDB35D51A4B6DDA4D3370DA67CE";
    uint32_t hex_gcm4lea_PP_byte_length = (uint32_t)(sizeof(hex_gcm4lea_PP)-1);

    uint8_t hex_gcm4lea_CC[] = "7A1C42F6982E60BD064F35029AD2BB36374515861660199C75F7F1710A9409E97CA7539999F1F23C037F17D2A6A90FB857D2498B78C52E41D4975888F5C85F40609DF0310F182CCF9ACDA0E816743BA0854F1ADE1802291958B58B8075426026D4C3B736818938BB494CEE430D5F938B4C089E853C8F18890B68D16A06E6B360";
    uint32_t hex_gcm4lea_CC_byte_length = (uint32_t)(sizeof(hex_gcm4lea_CC)-1);

    uint8_t hex_gcm4lea_T[]  = "CFBE61A72C74737CD3E955DFE172E991";
    uint32_t hex_gcm4lea_T_byte_length = (uint32_t)(sizeof(hex_gcm4lea_T)-1);

#elif defined(_TEST_3_)
    uint32_t key_bit_length = 256;
    uint8_t hex_gcm4lea_K[]  = "4EA55980F5CBD779F5482E4E29D9354B37DDDF6BC2B90DD6E17CA6A8D0BFEAF4";
    uint32_t hex_gcm4lea_K_byte_length = (uint32_t)(sizeof(hex_gcm4lea_K)-1);

    uint8_t hex_gcm4lea_N[]  = "E6D00309C70FDDF3DB973845";
    uint32_t hex_gcm4lea_N_byte_length = (uint32_t)(sizeof(hex_gcm4lea_N)-1);

    uint8_t hex_gcm4lea_A[]  = "FDD0F1303FA1FECC5CAFDF46D6CA358C71F118233070AD6B7F4BFB23B2DFFA2FAEC7ED99E6FDF2D78A41D84CFABF301181ECA7F440B193C168CBCBC1AD72BC81F738B9D45A35931B48616AC40541C2115CC6D91B9172391696BC89D681B6A8923EEDFDE981900822FFD53DB8DE9913166A47589F09FAC7F171E6DC6B18F4E7ED";
    uint32_t hex_gcm4lea_A_byte_length = (uint32_t)(sizeof(hex_gcm4lea_A)-1);

    uint8_t hex_gcm4lea_PP[] = "FF21FC167A26CC4A3244DD1320E309B601181F934ECA7E606F13E2B52CE87DCD64FD4A8B01E39EBB2B73009BBC703A99F468B05F6FDBF10D53A08CC5CE83256DC033C7B0B45C7C4615101B769F6B62D9137744602F4D8498DD7D832DABB057AE7B8C1C8F7BD88F7458E1D9ADAF1CA8FCC50C849F7766618A747470F5A9B73B16";
    uint32_t hex_gcm4lea_PP_byte_length = (uint32_t)(sizeof(hex_gcm4lea_PP)-1);

    uint8_t hex_gcm4lea_CC[] = "48D6C7AC5D5F4667BAEB65BA6E43587BB7ACFFFB6C7FF1F783887B9E1E4C708AF55907A2CEB8B6591B8B0D6A012CBDBCCBB64CBC3663E3F333C94351AD2AD879C76BC17568B31F79D4BE6D701CAF17288B3C95757A08DE4CBEB8962D0DF89076D14D3A94FC3ACD49D916324A6AB428CFA3324B4BF616716B477D00B1A0F2C300";
    uint32_t hex_gcm4lea_CC_byte_length = (uint32_t)(sizeof(hex_gcm4lea_CC)-1);

    uint8_t hex_gcm4lea_T[]  = "E1FDCD5EE21B5D0D165041AF47B812BE";
    uint32_t hex_gcm4lea_T_byte_length = (uint32_t)(sizeof(hex_gcm4lea_T)-1);

#elif defined(_TEST_4_)
    uint32_t key_bit_length = 128;
    uint8_t hex_gcm4lea_K[]  = "277D6A496ABCDCA578669A7B10975E64";
    uint32_t hex_gcm4lea_K_byte_length = (uint32_t)(sizeof(hex_gcm4lea_K)-1);

    uint8_t hex_gcm4lea_N[]  = "F2591F51DCC59DCFB86B3A8A";
    uint32_t hex_gcm4lea_N_byte_length = (uint32_t)(sizeof(hex_gcm4lea_N)-1);

    uint8_t hex_gcm4lea_A[]  = "ABBA1DB55527CFF2E7E099F1131BEC2DFFB53F11F918620067620F98FCC4C3383227CBBFF416A2979AD87AF31BDC19547E5BE65292384499DF2B1E32DC1EC30463D7F59A61D83C26D28381C6ED82D7079B669CA82E8EC1A0504B66613941AA5F2611C1CC063547A6762BD3F4EFD5D04D3E20889935E4821CA10B0EB07CF61E51";
    uint32_t hex_gcm4lea_A_byte_length = (uint32_t)(sizeof(hex_gcm4lea_A)-1);

    uint8_t hex_gcm4lea_PP[] = "14F40B10D2062FD821685F83AC67C76111781437941ECF40150EE9713F929701032B6029447A7329D4603F8BA0653EAB275611773132D274BD48A6023B9991E0FB838FF2E6A1E0426CEDA346BD27A6613C7B7DAB305ED1F5BDB9FB081449D12EE645C0F41E421C2DCF55B33A6B77A889372D7F5CFB6874CCFEA9110D316AFFF3";
    uint32_t hex_gcm4lea_PP_byte_length = (uint32_t)(sizeof(hex_gcm4lea_PP)-1);

    uint8_t hex_gcm4lea_CC[] = "DEF2DBD9A5EC6BA875221DC55DABF94E78074D87617560A9E826C8CDC1E7C077F82AB1DA764CEBB75EA55E97EBF4A8BF897C6DE2854A35AFD894550E31BB9E2336DA01C526988CEB2AC74EA1D58964BE6E7BB7483CB87886A6FEDFFE84DF30EA1F96DDA0D6B45650926E000BD3B05A2C5A5BD0F117F172205182B74D32BE87F2";
    uint32_t hex_gcm4lea_CC_byte_length = (uint32_t)(sizeof(hex_gcm4lea_CC)-1);

    uint8_t hex_gcm4lea_T[]  = "6564DB97FE247E33633B9DC728826094";
    uint32_t hex_gcm4lea_T_byte_length = (uint32_t)(sizeof(hex_gcm4lea_T)-1);

#elif defined(_TEST_5_)
    uint32_t key_bit_length = 192;
    uint8_t hex_gcm4lea_K[]  = "4AB4E76E29F5CB0EA3B43FD61B1DD7DED9A53CF5FF35C7E5";
    uint32_t hex_gcm4lea_K_byte_length = (uint32_t)(sizeof(hex_gcm4lea_K)-1);

    uint8_t hex_gcm4lea_N[]  = "BE8A4EF8C9DC3EBA0ABDEA01";
    uint32_t hex_gcm4lea_N_byte_length = (uint32_t)(sizeof(hex_gcm4lea_N)-1);

    uint8_t hex_gcm4lea_A[]  = "5BBB63857E6A510A464A4A948633D5630A0F4254CE3F83636E3A3D6EDE771F3C5B8E73BDA19AF2935FE1DDD2E8B749042B9FBB71E5B4F49D15470E11B1CAC85E97EC1F60C6061EC0CEB6F6BC85C9512BFC5FE4BBB149437E06B6FAA5F20FD98BF71F8FF554777B987B11BEDBC53395DF6596E6BC9BC8180E2ACF28B10B0FF9C8";
    uint32_t hex_gcm4lea_A_byte_length = (uint32_t)(sizeof(hex_gcm4lea_A)-1);

    uint8_t hex_gcm4lea_PP[] = "9545232BDA6C09A71EB720B72C17A773470E2512641344659638E2A4CA0A666DB4B8500A097815F3EF272993B22F5D4FB8AE6BC5B7D5CF51258ED9D6F06101BB70987A339AA10AC276B518FDD3B70791395FC2861F9798C55E254BD8E68B63F2AF2BC82FF3AF901C9BA8167AF7754C3FB16752DE042347C829475F331250351A";
    uint32_t hex_gcm4lea_PP_byte_length = (uint32_t)(sizeof(hex_gcm4lea_PP)-1);

    uint8_t hex_gcm4lea_CC[] = "94683201F48F55A1A842D1B7D6B68625C3158C7653557C9DEE22D6A6AF2E9540C525B5528ED7B04F59B2F622188F872285D040A179CDC979F869B509983139F18F9116F2C4FFA75084E77547A670BE3C54079A264BAA0F8D878E03BBD210E6453622668059377FD8126B986D7D959D0FA248F05B97B73ACD783D2D99047B5A2B";
    uint32_t hex_gcm4lea_CC_byte_length = (uint32_t)(sizeof(hex_gcm4lea_CC)-1);

    uint8_t hex_gcm4lea_T[]  = "C5004054B52F878D556D31EF779CD6C2";
    uint32_t hex_gcm4lea_T_byte_length = (uint32_t)(sizeof(hex_gcm4lea_T)-1);

#elif defined(_TEST_6_)
    uint32_t key_bit_length = 256;
    uint8_t hex_gcm4lea_K[]  = "ECAC60DE45587A741866FA2F6D0F4AA71336C988929366DB61106D875973A017";
    uint32_t hex_gcm4lea_K_byte_length = (uint32_t)(sizeof(hex_gcm4lea_K)-1);

    uint8_t hex_gcm4lea_N[]  = "6A3E8ABD8DD31097F9DC4932";
    uint32_t hex_gcm4lea_N_byte_length = (uint32_t)(sizeof(hex_gcm4lea_N)-1);

    uint8_t hex_gcm4lea_A[]  = "9AF3584EAEBA401EE652F1D98549D5184DA5E6B737CA08DC689E9C3461EA528076043E5181AB2F0FD7323C407BE19F079298C7EB17B5A8DAD038566E8125D26F5F2A2398C782A99E3019AF6E5FF13FB6DAC43AA89EEBA8FB147E4C3242BD7B203EAE2FACE90858D2D95072EA97425C2D0B7268753433AF441DB8A7080C79F79D";
    uint32_t hex_gcm4lea_A_byte_length = (uint32_t)(sizeof(hex_gcm4lea_A)-1);

    uint8_t hex_gcm4lea_PP[] = "9AEA904DBF5CD7DF13833E5BCE04E4322C7DFC991F6B248E404BF3150CFFD1C84D740C8ED68228AB731376F92088E874795823FC72FAD65748064A76B8973BF96EF3E7F5C06E64F49C8B363DBF652156284D3CC8CDB547228C4D3E40646B2ECDE5B04807E5E934C27533A8B0146338DF22A47083976220F6F467F7FD7A44534D";
    uint32_t hex_gcm4lea_PP_byte_length = (uint32_t)(sizeof(hex_gcm4lea_PP)-1);

    uint8_t hex_gcm4lea_CC[] = "78F5D56FDDF65F07ED01728181C1928E390DB975FC38578492A770768A722A3465E46C4F257856A39FAD8CF50FD775AD4EF7809F547B413F18481839BD93CFDB2A8E995C957801A76C405DA6C23CFB80CF678075616D28B6D3021B275D5B676CA6EA515CD9FAB3FC66AB1BB023E1B9FFF8E4B71C32B39E3528506FD26D70491F";
    uint32_t hex_gcm4lea_CC_byte_length = (uint32_t)(sizeof(hex_gcm4lea_CC)-1);

    uint8_t hex_gcm4lea_T[]  = "5FE498DD6F1017DFFC118780529D3221";
    uint32_t hex_gcm4lea_T_byte_length = (uint32_t)(sizeof(hex_gcm4lea_T)-1);

    #endif


    uint8_t K[64] = {0, };
    uint8_t PP[MAX_P_C_BYTE_LENGTH] = {0, };
    uint8_t CC[MAX_P_C_BYTE_LENGTH] = {0, };
    uint8_t N[MAX_N_BYTE_LENGTH] = {0, };
    uint8_t A[MAX_A_BYTE_LENGTH] = {0, };
    uint8_t T[16] = {0, };

    uint32_t K_byte_length;
    uint32_t PP_byte_length;
    uint32_t CC_byte_length;
    uint32_t N_byte_length;
    uint32_t A_byte_length;
    uint32_t T_byte_length;
    uint32_t T_bit_length;

    K_byte_length 	= hexs2bytes(K, hex_gcm4lea_K, hex_gcm4lea_K_byte_length);
    N_byte_length 	= hexs2bytes(N, hex_gcm4lea_N, hex_gcm4lea_N_byte_length);
    A_byte_length 	= hexs2bytes(A, hex_gcm4lea_A, hex_gcm4lea_A_byte_length);
    PP_byte_length 	= hexs2bytes(PP, hex_gcm4lea_PP, hex_gcm4lea_PP_byte_length);
    CC_byte_length 	= hexs2bytes(CC, hex_gcm4lea_CC, hex_gcm4lea_CC_byte_length);
    T_byte_length 	= hexs2bytes(T, hex_gcm4lea_T, hex_gcm4lea_T_byte_length);
    T_bit_length    = (T_byte_length << 3);
    
    if((K_byte_length << 3) != key_bit_length)
	{
		CONSOL_PRINTF("\nERROR! --> Please check the key length");
		return -1;
	}


    // Initializing (w/ LEA Round Key Schedule)
    start[0] = ARM_CM_DWT_CYCCNT;
    ret = GCM4LEA_set_init_params(&gcm, K, key_bit_length, A, A_byte_length, T_bit_length);
    stop[0] = ARM_CM_DWT_CYCCNT;

    if(ret < 0)
    {
    	CONSOL_PRINTF("\nERROR! --> Encryption test failed. @GCM4LEA_set_init_params(%d)\n", ret);
    	return -1;
	}

    ret = GCM4LEA_set_enc_params(&gcm, PP, PP_byte_length, N, N_byte_length);
    if(ret < 0)
	{
		CONSOL_PRINTF("\nERROR! --> Encryption test failed. @GCM4LEA_set_enc_params(%d)\n", ret);
    	return -1;
	}


    // Encryption
    start[1] = ARM_CM_DWT_CYCCNT;
    ret = GCM4LEA_enc(&gcm);
    stop[1] = ARM_CM_DWT_CYCCNT;

    if(ret < 0)
    {
		CONSOL_PRINTF("\nERROR! --> Encryption test failed. @GCM4LEA_enc(%d)\n", ret);
    	return -1;
	}

    if(gcm.CC_byte_length != gcm.PP_byte_length)
	{
		CONSOL_PRINTF("[Input plain text]: ");
		for (i = 0; i < gcm.PP_byte_length; i++)
		{
			if (i % 4 == 0)
			{
				CONSOL_PRINTF(" ");
				if (i % 16 == 0) CONSOL_PRINTF("\n");
			}
			CONSOL_PRINTF("%02x", gcm.PP[i]);
		}
		CONSOL_PRINTF("\n\n");

		CONSOL_PRINTF("[Encrypted text output]: ");
		for (i = 0; i < gcm.CC_byte_length; i++)
		{
				if (i % 4 == 0)
				{
					CONSOL_PRINTF(" ");
					if (i % 16 == 0) CONSOL_PRINTF("\n");
				}
				CONSOL_PRINTF("%02x", gcm.CC[i]);
		}
		CONSOL_PRINTF("\n");

		CONSOL_PRINTF("\nERROR! --> Encryption test failed. (gcm.CC_byte_length != gcm.PP_byte_length)\n");
		return -1;
	}

    if(memcmp(CC, gcm.CC, gcm.CC_byte_length) != 0)
    {
    	CONSOL_PRINTF("[Given text encrypted]: ");
		for (i = 0; i < gcm.CC_byte_length; i++)
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
		for (i = 0; i < gcm.CC_byte_length; i++)
		{
				if (i % 4 == 0)
				{
					CONSOL_PRINTF(" ");
					if (i % 16 == 0) CONSOL_PRINTF("\n");
				}
				CONSOL_PRINTF("%02x", gcm.CC[i]);
		}
		CONSOL_PRINTF("\n\n");


		CONSOL_PRINTF("\nERROR! --> Encryption test failed. (Mismatch of encrypted text data)\n");
		return -1;
    }

    if(memcmp(T, gcm.T, (gcm.T_bit_length >> 3)) != 0)
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
			CONSOL_PRINTF("0x%02x ", gcm.T[i]);
		}
		CONSOL_PRINTF("\n");

		CONSOL_PRINTF("\nERROR! --> Encryption test failed. (Mismatch of T value)\n");
		return -1;
    }

    CONSOL_PRINTF("\n<<<<<<<<<<<<<<<  GCM4LEA Encryption test passed!!!  >>>>>>>>>>>>>>>\n");


    // Initialize again for Decryption Test!
    ret = GCM4LEA_set_init_params(&gcm, K, key_bit_length, A, A_byte_length, T_bit_length);
	if(ret < 0)
	{
		CONSOL_PRINTF("\nERROR! --> Decryption test failed. @GCM4LEA_set_init_params(%d)\n", ret);
		return -1;
	}

	ret = GCM4LEA_set_dec_params(&gcm, CC, CC_byte_length, N, N_byte_length, T);
	if(ret < 0)
	{
		CONSOL_PRINTF("\nERROR! --> Decryption test failed. @GCM4LEA_set_dec_params(%d)\n", ret);
		return -1;
	}


    // Decryption
    start[2] = ARM_CM_DWT_CYCCNT;
    ret = GCM4LEA_dec(&gcm);
    stop[2] = ARM_CM_DWT_CYCCNT;

    if(ret < 0)
    {
		CONSOL_PRINTF("[Generated T]: ");
		for(i = 0; i < 16; i++)
		{
			CONSOL_PRINTF("0x%02x ", gcm.T[i]);
		}
		CONSOL_PRINTF("\n");

		CONSOL_PRINTF("[Given T]: ");
		for(i = 0; i < 16; i++)
		{
			CONSOL_PRINTF("0x%02x ", T[i]);
		}
		CONSOL_PRINTF("\n\n");

		CONSOL_PRINTF("\nERROR! --> Decryption test failed. @GCM4LEA_dec (%d) - Mismatch of T value\n", ret);
		return -1;
    }

    if(gcm.PP_byte_length != gcm.CC_byte_length)
    {
    	CONSOL_PRINTF("[Input Encrypted text]: ");
		for (i = 0; i < gcm.CC_byte_length; i++)
		{
				if (i % 4 == 0)
				{
					CONSOL_PRINTF(" ");
					if (i % 16 == 0) CONSOL_PRINTF("\n");
				}
				CONSOL_PRINTF("%02x", gcm.CC[i]);
		}
		CONSOL_PRINTF("\n\n");

    	CONSOL_PRINTF("[Decrypted text output]: ");
		for (i = 0; i < gcm.PP_byte_length; i++)
		{
				if (i % 4 == 0)
				{
					CONSOL_PRINTF(" ");
					if (i % 16 == 0) CONSOL_PRINTF("\n");
				}
				CONSOL_PRINTF("%02x", gcm.PP[i]);
		}
		CONSOL_PRINTF("\n");

    	CONSOL_PRINTF("\nERROR! --> Decryption test failed. (gcm.PP_byte_length != gcm.CC_byte_length)\n");
        return -1;
    }

    if(memcmp(PP, gcm.PP, gcm.PP_byte_length) != 0)
    {
    	CONSOL_PRINTF("[Given plain text]: ");
		for (i = 0; i < gcm.PP_byte_length; i++)
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
		for (i = 0; i < gcm.PP_byte_length; i++)
		{
				if (i % 4 == 0)
				{
					CONSOL_PRINTF(" ");
					if (i % 16 == 0) CONSOL_PRINTF("\n");
				}
				CONSOL_PRINTF("%02x", gcm.PP[i]);
		}
		CONSOL_PRINTF("\n\n");

    	CONSOL_PRINTF("\nERROR! --> Decryption test failed. (Mismatch of decrypted text data)\n");
        return -1;
    }

	delta[0] = stop[0] - start[0];
	delta[1] = stop[1] - start[1];
	delta[2] = stop[2] - start[2];


    CONSOL_PRINTF("\n>>>>>>>>>>>>>>>  GCM4LEA Decryption test passed!!!  <<<<<<<<<<<<<<<\n\n");

    CONSOL_PRINTF("---------------------------------------------------------------\n");

    CONSOL_PRINTF("  Length of Params:\n");
    CONSOL_PRINTF("    - Key          = %d (bits)\n", (int)key_bit_length);
    CONSOL_PRINTF("    - P, C         = %d (bytes)\n", (int)PP_byte_length);
    CONSOL_PRINTF("    - N            = %d (bytes)\n", (int)N_byte_length);
    CONSOL_PRINTF("    - A            = %d (bytes)\n", (int)A_byte_length);
    CONSOL_PRINTF("    - T            = %d (bits)\n", (int)T_bit_length);

    CONSOL_PRINTF("\n  Elapsed Cycles:\n");
    CONSOL_PRINTF("    - Initializing = %d (cycles)\n", (int)delta[0]);
    CONSOL_PRINTF("    - Encryption   = %d (cycles), %f (cycle/byte)\n", (int)delta[1], ((float)delta[1] / (float)PP_byte_length));
    CONSOL_PRINTF("    - Decryption   = %d (cycles), %f (cycle/byte)\n", (int)delta[2], ((float)delta[2] / (float)PP_byte_length));

    CONSOL_PRINTF("---------------------------------------------------------------\n");

    return 0;
}
// End of gcm4lea_testbench.c
