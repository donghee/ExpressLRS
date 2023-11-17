// GCM mode of operation for LEA
//
// by Joungil Yun (sigipus@etri.re.kr)
//
// 2023.09.

#include "gcm4lea.h"


static uint8_t T_h8[256][16];

const uint8_t T_red8[256][2] = {
	{ 0x00, 0x00 }, { 0x01, 0xc2 }, { 0x03, 0x84 }, { 0x02, 0x46 }, { 0x07, 0x08 }, { 0x06, 0xca }, { 0x04, 0x8c }, { 0x05, 0x4e },
	{ 0x0e, 0x10 }, { 0x0f, 0xd2 }, { 0x0d, 0x94 }, { 0x0c, 0x56 }, { 0x09, 0x18 }, { 0x08, 0xda }, { 0x0a, 0x9c }, { 0x0b, 0x5e },
	{ 0x1c, 0x20 }, { 0x1d, 0xe2 }, { 0x1f, 0xa4 }, { 0x1e, 0x66 }, { 0x1b, 0x28 }, { 0x1a, 0xea }, { 0x18, 0xac }, { 0x19, 0x6e },
	{ 0x12, 0x30 }, { 0x13, 0xf2 }, { 0x11, 0xb4 }, { 0x10, 0x76 }, { 0x15, 0x38 }, { 0x14, 0xfa }, { 0x16, 0xbc }, { 0x17, 0x7e },
	{ 0x38, 0x40 }, { 0x39, 0x82 }, { 0x3b, 0xc4 }, { 0x3a, 0x06 }, { 0x3f, 0x48 }, { 0x3e, 0x8a }, { 0x3c, 0xcc }, { 0x3d, 0x0e },
	{ 0x36, 0x50 }, { 0x37, 0x92 }, { 0x35, 0xd4 }, { 0x34, 0x16 }, { 0x31, 0x58 }, { 0x30, 0x9a }, { 0x32, 0xdc }, { 0x33, 0x1e },
	{ 0x24, 0x60 }, { 0x25, 0xa2 }, { 0x27, 0xe4 }, { 0x26, 0x26 }, { 0x23, 0x68 }, { 0x22, 0xaa }, { 0x20, 0xec }, { 0x21, 0x2e },
	{ 0x2a, 0x70 }, { 0x2b, 0xb2 }, { 0x29, 0xf4 }, { 0x28, 0x36 }, { 0x2d, 0x78 }, { 0x2c, 0xba }, { 0x2e, 0xfc }, { 0x2f, 0x3e },
	{ 0x70, 0x80 }, { 0x71, 0x42 }, { 0x73, 0x04 }, { 0x72, 0xc6 }, { 0x77, 0x88 }, { 0x76, 0x4a }, { 0x74, 0x0c }, { 0x75, 0xce },
	{ 0x7e, 0x90 }, { 0x7f, 0x52 }, { 0x7d, 0x14 }, { 0x7c, 0xd6 }, { 0x79, 0x98 }, { 0x78, 0x5a }, { 0x7a, 0x1c }, { 0x7b, 0xde },
	{ 0x6c, 0xa0 }, { 0x6d, 0x62 }, { 0x6f, 0x24 }, { 0x6e, 0xe6 }, { 0x6b, 0xa8 }, { 0x6a, 0x6a }, { 0x68, 0x2c }, { 0x69, 0xee },
	{ 0x62, 0xb0 }, { 0x63, 0x72 }, { 0x61, 0x34 }, { 0x60, 0xf6 }, { 0x65, 0xb8 }, { 0x64, 0x7a }, { 0x66, 0x3c }, { 0x67, 0xfe },
	{ 0x48, 0xc0 }, { 0x49, 0x02 }, { 0x4b, 0x44 }, { 0x4a, 0x86 }, { 0x4f, 0xc8 }, { 0x4e, 0x0a }, { 0x4c, 0x4c }, { 0x4d, 0x8e },
	{ 0x46, 0xd0 }, { 0x47, 0x12 }, { 0x45, 0x54 }, { 0x44, 0x96 }, { 0x41, 0xd8 }, { 0x40, 0x1a }, { 0x42, 0x5c }, { 0x43, 0x9e },
	{ 0x54, 0xe0 }, { 0x55, 0x22 }, { 0x57, 0x64 }, { 0x56, 0xa6 }, { 0x53, 0xe8 }, { 0x52, 0x2a }, { 0x50, 0x6c }, { 0x51, 0xae },
	{ 0x5a, 0xf0 }, { 0x5b, 0x32 }, { 0x59, 0x74 }, { 0x58, 0xb6 }, { 0x5d, 0xf8 }, { 0x5c, 0x3a }, { 0x5e, 0x7c }, { 0x5f, 0xbe },
	{ 0xe1, 0x00 }, { 0xe0, 0xc2 }, { 0xe2, 0x84 }, { 0xe3, 0x46 }, { 0xe6, 0x08 }, { 0xe7, 0xca }, { 0xe5, 0x8c }, { 0xe4, 0x4e },
	{ 0xef, 0x10 }, { 0xee, 0xd2 }, { 0xec, 0x94 }, { 0xed, 0x56 }, { 0xe8, 0x18 }, { 0xe9, 0xda }, { 0xeb, 0x9c }, { 0xea, 0x5e },
	{ 0xfd, 0x20 }, { 0xfc, 0xe2 }, { 0xfe, 0xa4 }, { 0xff, 0x66 }, { 0xfa, 0x28 }, { 0xfb, 0xea }, { 0xf9, 0xac }, { 0xf8, 0x6e },
	{ 0xf3, 0x30 }, { 0xf2, 0xf2 }, { 0xf0, 0xb4 }, { 0xf1, 0x76 }, { 0xf4, 0x38 }, { 0xf5, 0xfa }, { 0xf7, 0xbc }, { 0xf6, 0x7e },
	{ 0xd9, 0x40 }, { 0xd8, 0x82 }, { 0xda, 0xc4 }, { 0xdb, 0x06 }, { 0xde, 0x48 }, { 0xdf, 0x8a }, { 0xdd, 0xcc }, { 0xdc, 0x0e },
	{ 0xd7, 0x50 }, { 0xd6, 0x92 }, { 0xd4, 0xd4 }, { 0xd5, 0x16 }, { 0xd0, 0x58 }, { 0xd1, 0x9a }, { 0xd3, 0xdc }, { 0xd2, 0x1e },
	{ 0xc5, 0x60 }, { 0xc4, 0xa2 }, { 0xc6, 0xe4 }, { 0xc7, 0x26 }, { 0xc2, 0x68 }, { 0xc3, 0xaa }, { 0xc1, 0xec }, { 0xc0, 0x2e },
	{ 0xcb, 0x70 }, { 0xca, 0xb2 }, { 0xc8, 0xf4 }, { 0xc9, 0x36 }, { 0xcc, 0x78 }, { 0xcd, 0xba }, { 0xcf, 0xfc }, { 0xce, 0x3e },
	{ 0x91, 0x80 }, { 0x90, 0x42 }, { 0x92, 0x04 }, { 0x93, 0xc6 }, { 0x96, 0x88 }, { 0x97, 0x4a }, { 0x95, 0x0c }, { 0x94, 0xce },
	{ 0x9f, 0x90 }, { 0x9e, 0x52 }, { 0x9c, 0x14 }, { 0x9d, 0xd6 }, { 0x98, 0x98 }, { 0x99, 0x5a }, { 0x9b, 0x1c }, { 0x9a, 0xde },
	{ 0x8d, 0xa0 }, { 0x8c, 0x62 }, { 0x8e, 0x24 }, { 0x8f, 0xe6 }, { 0x8a, 0xa8 }, { 0x8b, 0x6a }, { 0x89, 0x2c }, { 0x88, 0xee },
	{ 0x83, 0xb0 }, { 0x82, 0x72 }, { 0x80, 0x34 }, { 0x81, 0xf6 }, { 0x84, 0xb8 }, { 0x85, 0x7a }, { 0x87, 0x3c }, { 0x86, 0xfe },
	{ 0xa9, 0xc0 }, { 0xa8, 0x02 }, { 0xaa, 0x44 }, { 0xab, 0x86 }, { 0xae, 0xc8 }, { 0xaf, 0x0a }, { 0xad, 0x4c }, { 0xac, 0x8e },
	{ 0xa7, 0xd0 }, { 0xa6, 0x12 }, { 0xa4, 0x54 }, { 0xa5, 0x96 }, { 0xa0, 0xd8 }, { 0xa1, 0x1a }, { 0xa3, 0x5c }, { 0xa2, 0x9e },
	{ 0xb5, 0xe0 }, { 0xb4, 0x22 }, { 0xb6, 0x64 }, { 0xb7, 0xa6 }, { 0xb2, 0xe8 }, { 0xb3, 0x2a }, { 0xb1, 0x6c }, { 0xb0, 0xae },
	{ 0xbb, 0xf0 }, { 0xba, 0x32 }, { 0xb8, 0x74 }, { 0xb9, 0xb6 }, { 0xbc, 0xf8 }, { 0xbd, 0x3a }, { 0xbf, 0x7c }, { 0xbe, 0xbe },
};


// GHASH
// It processes 128-bit inputs, and previous values and initial values must be managed externally
void GHASH128(uint8_t *r, const uint8_t *x)
{
	register int i = 0;
	register uint8_t mask;

	uint8_t y[16] = {0, };
	uint8_t z[16] = {0, };

	//  Perform XOR by converting from 8 to 32 (for speed improvement)
	XOR32x4(y, r, x);

	memset(z, 0, 16);

	for (i = 15; i > 0; i--)
	{
		XOR32x4(z, z, T_h8[y[i]]);

		mask = z[15];
		SHIFT8x16_R8(z);
		z[0] ^= T_red8[mask][0];
		z[1] ^= T_red8[mask][1];
	}

	XOR32x4(r, z, T_h8[y[i]]);
}


// Generate GCM CTR0
int gen_GCM_CTR0(GCM_st *gcm)
{
	register int i = 0;
	register uint8_t *p_N;
	register uint32_t remain;

	uint8_t tmp_8x16[16];
	uint32_t tmp_32x4[4];

	if (gcm->N_byte_length != 12)
	{
		// If it is less than 16 at the end, it operates the same as inserting a zero because it is filled with zeros during the initialization process of gcm->N
		// Nevertheless, it is filled with zeros (no need to calculate s)
		remain = gcm->N_byte_length % 16;
		if(remain)
		{
			memset((gcm->N + gcm->N_byte_length), 0, (16 - remain));
		}

		p_N = gcm->N;
		// Here, after being initially processed to the same result as initializing GHASH internal Y to 0, GHASH is repeatedly called below
		memset(gcm->CTR0_8x16, 0, 16);

		for(i = gcm->N_byte_length; i > 0; i -= 16, p_N += 16)
		{
			GHASH128(gcm->CTR0_8x16, p_N);
		}

		memset(tmp_32x4, 0, 16);

		// [Caution!!!] It is assumed that the maximum value of N is limited to use as 32 bits, not 64 bits (needs to be modified if using up to 64 bits)
		tmp_32x4[3] = (gcm->N_byte_length << 3); // bit 길이

		word2byte(tmp_8x16, tmp_32x4, 16);

		GHASH128(gcm->CTR0_8x16, tmp_8x16);
	}
	else
	{
		memset(gcm->CTR0_8x16, 0, 16);

		memcpy(gcm->CTR0_8x16, gcm->N, 12);
		gcm->CTR0_8x16[15] = 1;
	}

	byte2word(gcm->CTR_32x4, gcm->CTR0_8x16, 16);
	return 0;
}

// Generate GCM T value
int gen_GCM_T(GCM_st *gcm)
{
	register int i = 0;
	register uint8_t *p_A;
	register uint8_t *p_CC;
	register uint32_t remain_b;
	register uint32_t remain;
	register uint32_t tmp_idx;

	uint32_t tmp_32x4[4];
	uint8_t tmp_8x16[16];

	memset(gcm->T, 0, 16);

	// If it is less than 16 at the end, it operates the same as inserting a zero because it is filled with zeros during the initialization process of gcm->A
	// Nevertheless, it is filled with zeros!
	remain = gcm->A_byte_length % 16;
	if(remain)
	{
		memset((gcm->A + gcm->A_byte_length), 0, (16 - remain));
	}

	p_A = gcm->A;
	for(i = gcm->A_byte_length; i > 0; i -= 16, p_A += 16)
	{
		GHASH128(gcm->T, p_A);
	}

	// To be modified!!!
	// [Note] If there is no change in the value of A, storing gcm->T here and reusing it in subsequent iterations could potentially improve performance


	// If it is less than 16 at the end, it operates the same as inserting a zero because it is filled with zeros during the initialization process of gcm->CC
	// Nevertheless, it is filled with zeros!
	remain = gcm->CC_byte_length % 16;
	if(remain)
	{
		memset((gcm->CC + gcm->CC_byte_length), 0, (16 - remain));
	}

	p_CC = gcm->CC;
	for(i = gcm->CC_byte_length; i > 0; i -= 16, p_CC += 16)
	{
		GHASH128(gcm->T, p_CC);
	}

	// [Caution!!!] It is assumed that the length of A and CC is limited to the extent that it can be represented in 32 bits
	tmp_32x4[0] = 0; // (uint32_t)(gcm->A_byte_length >> 29);
	tmp_32x4[1] = (uint32_t)(gcm->A_byte_length << 3);	// bit length
	tmp_32x4[2] = 0; // (uint32_t)(gcm->CC_byte_length >> 29);
	tmp_32x4[3] = (uint32_t)(gcm->CC_byte_length << 3);	// bit length

	word2byte(tmp_8x16, tmp_32x4, 16);
	GHASH128(gcm->T, tmp_8x16);

	XOR32x4(gcm->T, gcm->T, gcm->Y);

	remain_b = gcm->T_bit_length & 0x7;
	tmp_idx = (gcm->T_bit_length >> 3) - 1;
	if(remain_b)
	{
		gcm->T[tmp_idx] = gcm->T[tmp_idx] & (0xff << (8 - remain_b));
	}

	tmp_idx++; // [Note] Since 1 was subtracted earlier, even if there are no remaining bits, an increment of 1 is necessary.
	if(tmp_idx < 16) memset(gcm->T + tmp_idx, 0, (16 - tmp_idx));

	return 0;
}

// Reset the GCM data structure
//
// input : GCM_st
// output: GCM_st
// return: None
void GCM4LEA_reset(GCM_st *gcm)
{
	LEA_reset(&gcm->LEA);

	memset(gcm->T, 0, 16);
	gcm->T_bit_length = 0;

	memset(gcm->PP, 0, MAX_P_C_BYTE_LENGTH);
	gcm->PP_byte_length = 0;

	memset(gcm->CC, 0, MAX_P_C_BYTE_LENGTH);
	gcm->CC_byte_length = 0;

	memset(gcm->N, 0, MAX_N_BYTE_LENGTH);
	gcm->N_byte_length = 0;

	memset(gcm->A, 0, MAX_A_BYTE_LENGTH);
	gcm->A_byte_length = 0;

	memset(gcm->CTR_32x4, 0, 16);
	memset(gcm->CTR0_8x16, 0, 16);
	memset(gcm->H, 0, 16);
	memset(gcm->Y, 0, 16);
}


// Initialization for GCM
//
// Here, it sets parameters that do not change during the encryption and decryption process!!!
//
// input : GCM_st, key, key bit length, A, A byte length, T bit length
// output: GCM_st
// return: 0, -1(error, exceeded the maximum byte length of A), -2(error, exceeded the maximum bit length of T), -3(error, invalid key length)
int GCM4LEA_set_init_params(GCM_st *gcm, const uint8_t *key, const uint32_t key_bit_length, const uint8_t *A, const uint32_t A_byte_length, const uint32_t T_bit_length)
{
	LEA_st *lea = &gcm->LEA;

	register int remain;
	register int i = 0;
	register int j = 0;

	uint8_t tmp_H[16];

	if (A_byte_length > MAX_A_BYTE_LENGTH)	return -1;		// Maximum length exceeded!!!
	if (T_bit_length > 128)	return -2;						// Maximum length exceeded!!!

	GCM4LEA_reset(gcm);

	if(LEA_set_init_params(lea, key, key_bit_length) < 0) return -3;	// Invalid key length!!!

	memcpy(gcm->A, A, A_byte_length);
	gcm->A_byte_length = A_byte_length;
	remain = A_byte_length % 16;
	if(remain)
	{
		memset(gcm->A + A_byte_length, 0, (16 - remain));
	}

	gcm->T_bit_length = T_bit_length;

	memset(lea->P, 0, 16); 		// all zeros
	LEA_enc(lea);
	memcpy(gcm->H, lea->C, 16);	// H


	// LUT initialization for GHASH
	memcpy(tmp_H, gcm->H, 16);
	memcpy(T_h8[0x80], tmp_H, 16);
	for (i = 0x40; i >= 1; i >>= 1)
	{
		SHIFT8x16_R1(tmp_H);
		if (T_h8[i << 1][15] & 1)
		{
			tmp_H[0] ^= 0xe1;
		}
		memcpy(T_h8[i], tmp_H, 16);
	}

	for (i = 2; i < 256; i <<= 1)
	{
		for (j = 1; j < i; j++)
			XOR32x4(T_h8[i + j], T_h8[i], T_h8[j]);
	}

	return 0;
}


int GCM4LEA_set_enc_params(GCM_st *gcm, const uint8_t *PP, const uint32_t PP_byte_length, const uint8_t *N, const uint32_t N_byte_length)
{
	register int remain;

	if (PP_byte_length > MAX_P_C_BYTE_LENGTH) return -1;	// Maximum length exceeded!!!
	if (N_byte_length > MAX_N_BYTE_LENGTH) return -2;		// Maximum length exceeded!!!
	if (N_byte_length == 0) return -3;						// Invalid N byte length

	memcpy(gcm->PP, PP, PP_byte_length);
	gcm->PP_byte_length = PP_byte_length;
	remain = PP_byte_length % 16;
	if(remain)
	{
		memset(gcm->PP + PP_byte_length, 0, (16 - remain));
	}

	memcpy(gcm->N, N, N_byte_length);
	gcm->N_byte_length = N_byte_length;
	remain = N_byte_length % 16;
	if(remain)
	{
		memset(gcm->N + N_byte_length, 0, (16 - remain));
	}

	return 0;
}


int GCM4LEA_set_dec_params(GCM_st *gcm, const uint8_t *CC, const uint32_t CC_byte_length, const uint8_t *N, const uint32_t N_byte_length, const uint8_t *T)
{
	register int remain;

	if (CC_byte_length > MAX_P_C_BYTE_LENGTH) return -1;	// Maximum length exceeded!!!
	if (N_byte_length > MAX_N_BYTE_LENGTH) return -2;		// Maximum length exceeded!!!

	memcpy(gcm->CC, CC, CC_byte_length);
	gcm->CC_byte_length = CC_byte_length;
	remain = CC_byte_length % 16;
	if(remain)
	{
		memset(gcm->CC + CC_byte_length, 0, (16 - remain));
	}

	memcpy(gcm->N, N, N_byte_length);
	gcm->N_byte_length = N_byte_length;
	remain = N_byte_length % 16;
	if(remain)
	{
		memset(gcm->N + N_byte_length, 0, (16 - remain));
	}

	// [Note] Since it is a fixed short length, the entire initialization is performed.
	memset(gcm->T, 0, 16);
	memcpy(gcm->T, T, (gcm->T_bit_length >> 3));

	return 0;
}


// GCM4LEA encryption
//
// Before calling this function, the PP and N data must be copied to GCM_st from outside!!!
// For this, you can optionally use the 'GCM4LEA_set_enc_params' function,
// or you can set the data directly (which is faster).
//
// input : GCM_st
// output: GCM_st
// return: 0
int GCM4LEA_enc(GCM_st *gcm)
{
	LEA_st *lea = &gcm->LEA;
	register int i = 0;

	uint8_t CTR_8x16[16] = { 0, };
	register uint8_t *p_PP;
	register uint8_t *p_CC;
	register uint32_t remain;

	gen_GCM_CTR0(gcm);

	memcpy(lea->P, gcm->CTR0_8x16, 16);
	LEA_enc(lea);
	memcpy(gcm->Y, lea->C, 16);

	p_PP = gcm->PP;
	p_CC = gcm->CC;

	// [Caution!!!] Since it is assumed that the allocated memory length of "gcm->PP" is multiples of 16 and filled with an initial value of 0, the last block is not zero-padded.
	for(i = gcm->PP_byte_length; i > 0; i -= 16, p_PP += 16, p_CC += 16)
	{
		// [Caution!!!] It is assumed that the counter value, that is, the number of blocks is less than 0xFFFFFFFF
		gcm->CTR_32x4[3]++;

		word2byte(CTR_8x16, gcm->CTR_32x4, 16);
		memcpy(lea->P, CTR_8x16, 16);
		LEA_enc(lea);
		XOR32x4(p_CC, lea->C, p_PP);
	}

	remain = gcm->PP_byte_length % 16;
	if(remain)
	{
		p_CC = gcm->CC + gcm->PP_byte_length;
		memset(p_CC, 0, (16 - remain));
	}
	gcm->CC_byte_length = gcm->PP_byte_length;


	gen_GCM_T(gcm);

	return 0;
}


// GCM4LEA decryption
//
// Before calling this function, the CC, N and T data must be copied to GCM_st from outside!!!
// For this, you can optionally use the 'GCM4LEA_set_dec_params' function,
// or you can set the data directly (which is faster).
//
// input : GCM_st
// output: GCM_st
// return: 0, -1(error, mismatch of T)
int GCM4LEA_dec(GCM_st *gcm)
{
	register int i = 0;
	register uint8_t *p_PP;
	register uint8_t *p_CC;
	register uint32_t remain;

	LEA_st *lea = &gcm->LEA;
	uint8_t CTR_8x16[16] = { 0, };
	uint8_t T[16] = { 0, };


	gen_GCM_CTR0(gcm);
	memcpy(lea->P, gcm->CTR0_8x16, 16);
	LEA_enc(lea);
	memcpy(gcm->Y, lea->C, 16);

	memcpy(T, gcm->T, 16);

	// 예외 상황 대비용
	remain = gcm->CC_byte_length % 16;
	if(remain)
	{
		p_CC = gcm->CC + gcm->CC_byte_length;
		memset(p_CC, 0, (16 - remain));
	}

	gen_GCM_T(gcm);

	if(memcmp(T, gcm->T, (gcm->T_bit_length >> 3)) != 0)
	{
		return -1; // T value does not match!!!
	}

	p_CC = gcm->CC;
	p_PP = gcm->PP;

	// [Caution!!!] Since it is assumed that the allocated memory length of "gcm->CC" is multiples of 16 and filled with an initial value of 0, the last block is not zero-padded.
	for(i = gcm->CC_byte_length; i > 0; i -= 16, p_CC += 16, p_PP += 16)
	{
		// [Caution!!!] It is assumed that the counter value, that is, the number of blocks is less than 0xFFFFFFFF
		gcm->CTR_32x4[3]++;

		word2byte(CTR_8x16, gcm->CTR_32x4, 16);
		memcpy(lea->P, CTR_8x16, 16);
		LEA_enc(lea);
		XOR32x4(p_PP, lea->C, p_CC);
	}

	remain = gcm->CC_byte_length % 16;
	if(remain)
	{
		p_PP = gcm->PP + gcm->CC_byte_length;
		memset(p_PP, 0, (16 - remain));
	}
	gcm->PP_byte_length = gcm->CC_byte_length;

	return 0;
}
// End of gcm4lea.c
