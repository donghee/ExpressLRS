// GCM mode of operation for LEA
//
// by Joungil Yun (sigipus@etri.re.kr)
//
// 2023.09.

#ifndef __GCM4LEA_H__
#define __GCM4LEA_H__

#include "lea.h"

#define MAX_P_C_BYTE_LENGTH	256
#define MAX_N_BYTE_LENGTH	128
#define MAX_A_BYTE_LENGTH	128

typedef struct
{
	LEA_st 		LEA;							// for LEA enc/dec

	uint8_t 	T[16];						// Tag
	uint32_t 	T_bit_length;				// Tag bit length
	uint8_t 	N[MAX_N_BYTE_LENGTH];		// N (IV, nonce)
	uint32_t 	N_byte_length;				// N byte length
	uint8_t 	A[MAX_A_BYTE_LENGTH];		// AAD(Additional Authenticated Data)
	uint32_t 	A_byte_length;				// AAD byte length

	uint8_t 	PP[MAX_P_C_BYTE_LENGTH];	// Plain(Decrypted) text data
	uint32_t 	PP_byte_length;				// PP byte length
	uint8_t 	CC[MAX_P_C_BYTE_LENGTH];	// Encrypted(Received) text data
	uint32_t 	CC_byte_length;				// CC byte length (should be the same length as PP_byte_length)

	uint32_t 	CTR_32x4[4];				// Counter
	uint8_t 	CTR0_8x16[16];

	uint8_t 	H[16];						// H
	uint8_t 	Y[16];						// Y
} GCM_st;


// Reset GCM_st data structure. (clear all)
void GCM4LEA_reset(
		GCM_st *gcm
		);

// Optional Function
int GCM4LEA_set_enc_params(
		GCM_st *gcm,
		const uint8_t *PP,
		const uint32_t PP_byte_length,
		const uint8_t *N,
		const uint32_t N_byte_length
		);

// Optional Function
int GCM4LEA_set_dec_params(
		GCM_st *gcm,
		const uint8_t *CC,
		const uint32_t CC_byte_length,
		const uint8_t *N,
		const uint32_t N_byte_length,
		const uint8_t *T
		);

// Mandatory Function: Call Order 1
int GCM4LEA_set_init_params(
		GCM_st *gcm,
		const uint8_t *key,
		const uint32_t key_bit_length,
		const uint8_t *A,
		const uint32_t A_byte_length,
		const uint32_t T_bit_length
		);

// Mandatory Function: Call Order 2
int GCM4LEA_enc(
		GCM_st *gcm
		);

// Mandatory Function: Call Order 2
int GCM4LEA_dec(
		GCM_st *gcm
		);

#endif //__GCM4LEA_H__
// End of gcm4lea.h
