// CCM mode of operation for LEA
//
// by Joungil Yun (sigipus@etri.re.kr)
//
// 2023.10.

#ifndef __CCM4LEA_H__
#define __CCM4LEA_H__

#include "lea.h"

#define MAX_P_C_BYTE_LENGTH	256
#define MAX_A_BYTE_LENGTH	128
#define MAX_B_BYTE_LENGTH	(MAX_A_BYTE_LENGTH + MAX_P_C_BYTE_LENGTH + 26)

typedef struct
{
	LEA_st 		LEA;						// for LEA enc/dec
	uint8_t 	T[16];						// Tag
	uint32_t 	T_byte_length;				// Tag byte length
	uint8_t 	N[16];						// N (IV, nonce)
	uint32_t 	N_byte_length;				// N byte length
	uint8_t 	A[MAX_A_BYTE_LENGTH];		// AAD(Additional Authenticated Data)
	uint32_t 	A_byte_length;				// AAD byte length

	uint8_t 	PP[MAX_P_C_BYTE_LENGTH];	// Plain(Decrypted) text data
	uint32_t 	PP_byte_length;				// PP byte length
	uint8_t 	CC[MAX_P_C_BYTE_LENGTH];	// Encrypted(Received) text data
	uint32_t 	CC_byte_length;				// CC byte length (should be the same length as PP_byte_length)

	uint32_t 	CTR_32x4[4];				// Counter
	uint8_t 	CTR0_8x16[16];

	uint8_t 	FlagB;						// FlagB
	uint8_t		FlagC;						// FlagC

	uint8_t 	S0[16];						// S0

	uint8_t		B[MAX_B_BYTE_LENGTH];		// B
	uint32_t	B_byte_length;				// B byte length
} CCM_st;


// Reset CCM_st data structure. (clear all)
void CCM4LEA_reset(
		CCM_st *ccm
		);


// Optional Function
int CCM4LEA_set_enc_params(
		CCM_st *ccm,
		const uint8_t *PP,
		const uint32_t PP_byte_length,
		const uint8_t *N,
		const uint32_t N_byte_length
		);

// Optional Function
int CCM4LEA_set_dec_params(
		CCM_st *ccm,
		const uint8_t *CC,
		const uint32_t CC_byte_length,
		const uint8_t *N,
		const uint32_t N_byte_length,
		const uint8_t *T
		);

// Mandatory Function: Call Order 1
int CCM4LEA_set_init_params(
		CCM_st *ccm,
		const uint8_t *key,
		const uint32_t key_bit_length,
		const uint8_t *A,
		const uint32_t A_byte_length,
		const uint32_t T_bit_length
		);

// Mandatory Function: Call Order 2
int CCM4LEA_enc(
		CCM_st *ccm
		);

// Mandatory Function: Call Order 2
int CCM4LEA_dec(
		CCM_st *ccm
		);

#endif //__CCM4LEA_H__
// End of ccm4lea.h
