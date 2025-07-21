// ASCON-128a
//
// by Joungil Yun (sigipus@etri.re.kr)
//
// 2024.07.

#include "ascon128x.h"

// Reset the ASCON data structure
//
// input : ASCON_st
// output: ASCON_st
// return: None
void ASCON128x_reset(ASCON_st *ascon)
{
	memset(ascon->K, 0, 16);
	ascon->K_byte_length = 16;

	memset(ascon->N, 0, 16);
	ascon->N_byte_length = 16;

	memset(ascon->T, 0, 16);
	ascon->T_byte_length = 0;

	memset(ascon->PP, 0, MAX_P_C_BYTE_LENGTH);
	ascon->PP_byte_length = 0;

	memset(ascon->CC, 0, MAX_P_C_BYTE_LENGTH);
	ascon->CC_byte_length = 0;

	memset(ascon->A, 0, MAX_A_BYTE_LENGTH);
	ascon->A_byte_length = 0;
}


// Initialization for ASCON
//
// Here, it sets parameters that do not change during the encryption and decryption process!!!
//
// input : ASCON_st, key, key bit length, A, A byte length, T byte length
// output: ASCON_st
// return: 0, -1(error, exceeded the maximum length of A), -2(error, invalid T byte length), -3(error, invalid key length)
int ASCON128x_set_init_params(ASCON_st *ascon, const uint8_t *key, const uint32_t key_bit_length, const uint8_t *A, const uint32_t A_byte_length, const uint32_t T_byte_length)
{
	register int remain;

	if(A_byte_length > MAX_A_BYTE_LENGTH)	return -1;					// Maximum length exceeded!!!

	// [참고]ASCON 규격은 128 bits이지만 임의 byte(최대는 16) 길이를 사용할 예정임
	if(T_byte_length < 1 || T_byte_length > 16) return -2;				// Invalid T byte length!!!

	ASCON128x_reset(ascon);

	if(key_bit_length != 128)	return -3; // Invalid key length!!!

	memcpy(ascon->K, key, 16);
	ascon->K_byte_length = 16;

	memcpy(ascon->A, A, A_byte_length);
	ascon->A_byte_length = A_byte_length;
	remain = A_byte_length % 16;
	if(remain)
	{
		memset(ascon->A + A_byte_length, 0, (16 - remain));
	}

	ascon->T_byte_length = T_byte_length;

	return 0;
}


int ASCON128x_set_enc_params(ASCON_st *ascon, const uint8_t *PP, const uint32_t PP_byte_length, const uint8_t *N, const uint32_t N_byte_length)
{
	register int remain;

	if(PP_byte_length > MAX_P_C_BYTE_LENGTH) return -1;		// Maximum length exceeded!!!
	if(N_byte_length != 16) return -2;						// Invalid N byte length!!!

	memcpy(ascon->PP, PP, PP_byte_length);
	ascon->PP_byte_length = PP_byte_length;
	remain = PP_byte_length % 16;
	if(remain)
	{
		memset(ascon->PP + PP_byte_length, 0, (16 - remain));
	}
	memcpy(ascon->N, N, N_byte_length);
	ascon->N_byte_length = N_byte_length;

	return 0;
}


int ASCON128x_set_dec_params(ASCON_st *ascon, const uint8_t *CC, const uint32_t CC_byte_length, const uint8_t *N, const uint32_t N_byte_length, const uint8_t *T)
{
	register int remain;

	if(CC_byte_length > MAX_P_C_BYTE_LENGTH) return -1;		// Maximum length exceeded!!!
	if(N_byte_length != 16) return -2;						// Invalid N byte length!!!

	memcpy(ascon->CC, CC, CC_byte_length);
	ascon->CC_byte_length = CC_byte_length;
	remain = CC_byte_length % 16;
	if(remain)
	{
		memset(ascon->CC + CC_byte_length, 0, (16 - remain));
	}

	memcpy(ascon->N, N, N_byte_length);
	ascon->N_byte_length = N_byte_length;

	memset(ascon->T, 0, 16);
	memcpy(ascon->T, T, ascon->T_byte_length);

	return 0;
}




// ASCON-128a encryption
//
// Before calling this function, the PP and N data must be copied to ASCON_st from outside!!!
// For this, you can optionally use the 'ASCON128x_set_enc_params' function,
// or you can set the data directly (which is faster).
//
// input : ASCON_st
// output: ASCON_st
// return: 0
int ASCON128x_enc(ASCON_st *ascon)
{
	ascon_state_t s;
	ascon_key_t key;

	ascon_loadkey(&key, ascon->K);
	ascon_initaead(&s, &key, ascon->N);
	ascon_adata(&s, ascon->A, ascon->A_byte_length);
	ascon_encrypt(&s, ascon->CC, ascon->PP, ascon->PP_byte_length);
	ascon->CC_byte_length = ascon->PP_byte_length;
	ascon_final(&s, &key);
	ascon_gettag(&s, ascon->T);

	return 0;
}


// ASCON-128a decryption
//
// Before calling this function, the CC, N and T data must be copied to ASCON_st from outside!!!
// For this, you can optionally use the 'ASCON128x_set_dec_params' function,
// or you can set the data directly (which is faster).
//
// input : ASCON_st
// output: ASCON_st
// return: 0, -1(error, mismatch of T)
int ASCON128x_dec(ASCON_st *ascon)
{
	uint8_t T[16] = { 0, };

	ascon_state_t s;
	ascon_key_t key;

	ascon_loadkey(&key, ascon->K);
	ascon_initaead(&s, &key, ascon->N);
	ascon_adata(&s, ascon->A, ascon->A_byte_length);
	ascon_decrypt(&s, ascon->PP, ascon->CC, ascon->CC_byte_length);
	ascon->PP_byte_length = ascon->CC_byte_length;
	ascon_final(&s, &key);

	memcpy(T, ascon->T, ascon->T_byte_length);
	ascon_gettag(&s, ascon->T);

	if(memcmp(T, ascon->T, ascon->T_byte_length) != 0)
	{
		return -1; // Mismatch of T!!!
	}

	return 0;
}

// End of ascon128x.c
