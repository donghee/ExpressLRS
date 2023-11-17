// CCM mode of operation for LEA
//
// by Joungil Yun (sigipus@etri.re.kr)
//
// 2023.10.

#include "ccm4lea.h"


// FlagB parameters byte packing.
typedef union
{
    uint8_t v_8;
    struct
	{
    	uint8_t qRes_3 : 3;
    	uint8_t tRes_3 : 3;
    	uint8_t Adata_1 : 1;
    	uint8_t Zero_1 : 1;
    };
} FlagB_pack;


// FlagC parameters byte packing.
typedef union
{
    uint8_t v_8;
    struct
	{
    	uint8_t qRes_3 : 3;
    	uint8_t Zero_5 : 5;
    };
} FlagC_pack;


// Generate CCM CTR0
int gen_CCM_CTR0(CCM_st *ccm)
{
	FlagC_pack Fp;

	Fp.Zero_5 = 0;
	Fp.qRes_3 = 14 - ccm->N_byte_length; // q - 1 = (15 - n) - 1 = 14 - n
	ccm->FlagC = Fp.v_8;

	memset(ccm->CTR0_8x16, 0, 16);
	ccm->CTR0_8x16[0] = ccm->FlagC;
	memcpy((ccm->CTR0_8x16 + 1), ccm->N, ccm->N_byte_length);

	byte2word(ccm->CTR_32x4, ccm->CTR0_8x16, 16);

	return 0;
}


// Generate CCM T value
int gen_CCM_T(CCM_st *ccm)
{
	LEA_st *lea = &ccm->LEA;

	register int i = 0;
	register uint8_t *p_B;
	register uint32_t tmp_length;
	register uint32_t tmp_idx;
	register uint32_t remain;

	FlagB_pack Fb;

	Fb.Zero_1 = 0;
	Fb.Adata_1 = (ccm->A_byte_length != 0);
	Fb.tRes_3 = (ccm->T_byte_length - 2) >> 1;
	Fb.qRes_3 = 14 - ccm->N_byte_length;	// (15 - ccm->N_byte_length) - 1;

	ccm->FlagB = Fb.v_8;

	memset(ccm->T, 0, 16);

	p_B = ccm->B;

	//
	// B0
	//
	p_B[0] = ccm->FlagB;
	memcpy((p_B + 1), ccm->N, ccm->N_byte_length);

	tmp_length = ccm->PP_byte_length;
	for(i = 15; i > ccm->N_byte_length; i--)
	{
		ccm->B[i] = (uint8_t)(tmp_length & 0xFF);
		tmp_length = tmp_length >> 8;
	}

	if(ccm->A_byte_length > 0 && ccm->A_byte_length < 0xFF00)
	{
		ccm->B[16] = (uint8_t)((ccm->A_byte_length >> 8) & 0xFF);
		ccm->B[17] = (uint8_t)((ccm->A_byte_length     ) & 0xFF);

		tmp_idx = 18;
	}
	else if(ccm->A_byte_length >= 0xFF00 && ccm->A_byte_length <= 0xFFFFFFFF) // [Note] 0xFFFFFFFF = 2^32 - 1
	{
		ccm->B[16] = 0xFF;
		ccm->B[17] = 0xFE;

		ccm->B[18] = (uint8_t)((ccm->A_byte_length >> 24) & 0xFF);
		ccm->B[19] = (uint8_t)((ccm->A_byte_length >> 16) & 0xFF);
		ccm->B[20] = (uint8_t)((ccm->A_byte_length >>  8) & 0xFF);
		ccm->B[21] = (uint8_t)((ccm->A_byte_length      ) & 0xFF);

		tmp_idx = 22;
	}
	// [Caution!!!] It is assumed that this case does not occur.
	// If it is necessary to handle this case, the data type of ccm->A_byte_length must be changed!!!
	else if(ccm->A_byte_length > 0xFFFFFFFF)
	{
		ccm->B[16] = 0xFF;
		ccm->B[17] = 0xFF;

		ccm->B[18] = 0; // (uint8_t)((ccm->A_byte_length >> 56) & 0xFF);
		ccm->B[19] = 0; // (uint8_t)((ccm->A_byte_length >> 48) & 0xFF);
		ccm->B[20] = 0; // (uint8_t)((ccm->A_byte_length >> 40) & 0xFF);
		ccm->B[21] = 0; // (uint8_t)((ccm->A_byte_length >> 32) & 0xFF);
		ccm->B[22] = (uint8_t)((ccm->A_byte_length >> 24) & 0xFF);
		ccm->B[23] = (uint8_t)((ccm->A_byte_length >> 16) & 0xFF);
		ccm->B[24] = (uint8_t)((ccm->A_byte_length >>  8) & 0xFF);
		ccm->B[25] = (uint8_t)((ccm->A_byte_length      ) & 0xFF);

		tmp_idx = 26;
	}
	else // No input for A (not used)!!!
	{
		tmp_idx = 16;
	}

	//
	// Attach A
	//
	memcpy((p_B + tmp_idx), ccm->A, ccm->A_byte_length);
	tmp_idx = tmp_idx + ccm->A_byte_length;

	remain = tmp_idx % 16;
	if(remain)
	{
		memset((p_B + tmp_idx), 0, 16 - remain);
		tmp_idx = tmp_idx + (16 - remain);
	}

	//
	// Attach PP
	//
	memcpy((p_B + tmp_idx), ccm->PP, ccm->PP_byte_length);
	tmp_idx = tmp_idx + ccm->PP_byte_length;

	remain = tmp_idx % 16;
	if(remain)
	{
		memset((p_B + tmp_idx), 0, 16 - remain);
		tmp_idx = tmp_idx + (16 - remain);
	}

	ccm->B_byte_length = tmp_idx;


	// Generate T (based on CBC-MAC)
	memcpy(lea->P, p_B, 16);
	LEA_enc(lea);
	p_B += 16;
	for(i = ccm->B_byte_length - 16; i > 0; i -= 16, p_B += 16)
	{
		XOR32x4(lea->P, lea->C, p_B);
		LEA_enc(lea);
	}

	XOR32x4(ccm->T, lea->C, ccm->S0);

	if(ccm->T_byte_length < 16) memset(ccm->T + ccm->T_byte_length, 0, 16 - ccm->T_byte_length);

	return 0;
}



// Reset the CCM data structure
//
// input : CCM_st
// output: CCM_st
// return: None
void CCM4LEA_reset(CCM_st *ccm)
{
	LEA_reset(&ccm->LEA);

	memset(ccm->T, 0, 16);
	ccm->T_byte_length = 0;

	memset(ccm->PP, 0, MAX_P_C_BYTE_LENGTH);
	ccm->PP_byte_length = 0;

	memset(ccm->CC, 0, MAX_P_C_BYTE_LENGTH);
	ccm->CC_byte_length = 0;

	memset(ccm->N, 0, 16);
	ccm->N_byte_length = 0;

	memset(ccm->A, 0, MAX_A_BYTE_LENGTH);
	ccm->A_byte_length = 0;

	memset(ccm->CTR_32x4, 0, 16);
	memset(ccm->CTR0_8x16, 0, 16);

	ccm->FlagB = 0;
	ccm->FlagC = 0;

	memset(ccm->S0, 0, 16);
	memset(ccm->B, 0, MAX_A_BYTE_LENGTH + MAX_P_C_BYTE_LENGTH + 1);
}


// Initialization for CCM
//
// Here, it sets parameters that do not change during the encryption and decryption process!!!
//
// input : CCM_st, key, key bit length, A, A byte length, T byte length
// output: CCM_st
// return: 0, -1(error, exceeded the maximum length of A), -2(error, invalid T byte length), -3(error, invalid key length)
int CCM4LEA_set_init_params(CCM_st *ccm, const uint8_t *key, const uint32_t key_bit_length, const uint8_t *A, const uint32_t A_byte_length, const uint32_t T_byte_length)
{
	LEA_st *lea = &ccm->LEA;

	register int remain;

	if(A_byte_length > MAX_A_BYTE_LENGTH)	return -1;										// Maximum length exceeded!!!
	if((T_byte_length % 2 != 0) || (T_byte_length < 4 || T_byte_length > 16)) return -2;	// Invalid T byte length!!!

	CCM4LEA_reset(ccm);

	if(LEA_set_init_params(lea, key, key_bit_length) < 0)	return -3; // Invalid key length!!!

	memcpy(ccm->A, A, A_byte_length);
	ccm->A_byte_length = A_byte_length;
	remain = A_byte_length % 16;
	if(remain)
	{
		memset(ccm->A + A_byte_length, 0, (16 - remain));
	}

	ccm->T_byte_length = T_byte_length;

	return 0;
}


int CCM4LEA_set_enc_params(CCM_st *ccm, const uint8_t *PP, const uint32_t PP_byte_length, const uint8_t *N, const uint32_t N_byte_length)
{
	register int remain;

	if(PP_byte_length > MAX_P_C_BYTE_LENGTH) return -1;		// Maximum length exceeded!!!
	if(N_byte_length > 13 || N_byte_length < 7) return -2;	// Invalid N byte length!!!

	// Additional condition check for n + q = 15 is required!!!

	memcpy(ccm->PP, PP, PP_byte_length);
	ccm->PP_byte_length = PP_byte_length;
	remain = PP_byte_length % 16;
	if(remain)
	{
		memset(ccm->PP + PP_byte_length, 0, (16 - remain));
	}
	memcpy(ccm->N, N, N_byte_length);
	ccm->N_byte_length = N_byte_length;

	return 0;
}


int CCM4LEA_set_dec_params(CCM_st *ccm, const uint8_t *CC, const uint32_t CC_byte_length, const uint8_t *N, const uint32_t N_byte_length, const uint8_t *T)
{
	register int remain;

	if(CC_byte_length > MAX_P_C_BYTE_LENGTH) return -1;		// Maximum length exceeded!!!
	if(N_byte_length > 13 || N_byte_length < 7) return -2;	// Invalid N byte length!!!

	// Additional condition check for n + q = 15 is required!!!

	memcpy(ccm->CC, CC, CC_byte_length);
	ccm->CC_byte_length = CC_byte_length;
	remain = CC_byte_length % 16;
	if(remain)
	{
		memset(ccm->CC + CC_byte_length, 0, (16 - remain));
	}

	memcpy(ccm->N, N, N_byte_length);
	ccm->N_byte_length = N_byte_length;
	remain = N_byte_length % 16;
	if(remain)
	{
		memset(ccm->N + N_byte_length, 0, (16 - remain));
	}

	// [Note] Since it is a fixed short length, the entire initialization is performed.
	memset(ccm->T, 0, 16);
	memcpy(ccm->T, T, ccm->T_byte_length);

	return 0;
}


// CCM4LEA encryption
//
// Before calling this function, the PP and N data must be copied to CCM_st from outside!!!
// For this, you can optionally use the 'CCM4LEA_set_enc_params' function,
// or you can set the data directly (which is faster).
//
// input : CCM_st
// output: CCM_st
// return: 0
int CCM4LEA_enc(CCM_st *ccm)
{
	LEA_st *lea = &ccm->LEA;
	register int i = 0;
	register uint8_t *p_PP;
	register uint8_t *p_CC;
	register uint32_t remain;

	uint8_t CTR_8x16[16] = { 0, };

	gen_CCM_CTR0(ccm);
	memcpy(lea->P, ccm->CTR0_8x16, 16);
	LEA_enc(lea);
	memcpy(ccm->S0, lea->C, 16);

	p_PP = ccm->PP;
	p_CC = ccm->CC;

	// [Caution!!!] Since it is assumed that the allocated memory length of "ccm->PP" is multiples of 16 and filled with an initial value of 0, the last block is not zero-padded.
	for(i = ccm->PP_byte_length; i > 0; i -= 16, p_PP += 16, p_CC += 16)
	{
		// [Caution!!!] It is assumed that the counter value, that is, the number of blocks is less than 0xFFFFFFFF
		ccm->CTR_32x4[3]++;

		word2byte(CTR_8x16, ccm->CTR_32x4, 16);
		memcpy(lea->P, CTR_8x16, 16);
		LEA_enc(lea);
		XOR32x4(p_CC, lea->C, p_PP);
	}

	remain = ccm->PP_byte_length % 16;
	if(remain)
	{
		p_CC = ccm->CC + ccm->PP_byte_length;
		memset(p_CC, 0, (16 - remain));
	}
	ccm->CC_byte_length = ccm->PP_byte_length;

	gen_CCM_T(ccm);

	return 0;
}


// CCM4LEA decryption
//
// Before calling this function, the CC, N and T data must be copied to CCM_st from outside!!!
// For this, you can optionally use the 'CCM4LEA_set_dec_params' function,
// or you can set the data directly (which is faster).
//
// input : CCM_st
// output: CCM_st
// return: 0, -1(error, mismatch of T)
int CCM4LEA_dec(CCM_st *ccm)
{
	LEA_st *lea = &ccm->LEA;

	register int i = 0;
	register uint8_t *p_PP;
	register uint8_t *p_CC;
	register uint32_t remain;

	uint8_t CTR_8x16[16] = { 0, };
	uint8_t T[16] = { 0, };

	gen_CCM_CTR0(ccm);
	memcpy(lea->P, ccm->CTR0_8x16, 16);
	LEA_enc(lea);
	memcpy(ccm->S0, lea->C, 16);

	p_CC = ccm->CC;
	p_PP = ccm->PP;

	// [Caution!!!] Since it is assumed that the allocated memory length of "ccm->CC" is multiples of 16 and filled with an initial value of 0, the last block is not zero-padded.
	for(i = ccm->CC_byte_length; i > 0; i -= 16, p_CC += 16, p_PP += 16)
	{
		// [Caution!!!] It is assumed that the counter value, that is, the number of blocks is less than 0xFFFFFFFF
		ccm->CTR_32x4[3]++;

		word2byte(CTR_8x16, ccm->CTR_32x4, 16);
		memcpy(lea->P, CTR_8x16, 16);
		LEA_enc(lea);
		XOR32x4(p_PP, lea->C, p_CC);
	}

	remain = ccm->CC_byte_length % 16;
	if(remain)
	{
		p_PP = ccm->PP + ccm->CC_byte_length;
		memset(p_PP, 0, (16 - remain));
	}
	ccm->PP_byte_length = ccm->CC_byte_length;


	// [Note] Unlike GCM, CCM allows for T verification after decryption because plain text is input into the calculation of T
	memcpy(T, ccm->T, ccm->T_byte_length);
	gen_CCM_T(ccm);

	if(memcmp(T, ccm->T, ccm->T_byte_length) != 0)
	{
		return -1; // Mismatch of T!!!
	}

	return 0;
}

// End of ccm4lea.c
