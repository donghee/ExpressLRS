// LEA
//   - Round Key Scheduler
//   - Encryption
//   - Decryption
//
// by Joungil Yun (sigipus@etri.re.kr)
//
// 2023.09.

#ifndef __LEA_H__
#define __LEA_H__

#include "tools.h"

typedef struct
{
	uint8_t 	K[32];				// Master Key
	uint32_t 	K_byte_length;		// Master Key byte length
	uint32_t 	RK[192];			// Round Key array
	uint32_t 	Nr;					// Number of Round
	uint8_t 	P[16];				// Plain text 128 bits
	uint8_t 	C[16];				// Encrypted text 128 bits
} LEA_st;


void LEA_reset(
		LEA_st *lea
		);

// Mandatory Call Function: Call Order 1
int LEA_set_init_params(
		LEA_st *lea,
		const uint8_t *key,
		const uint32_t key_bit_length 	// bit length of master key (128/192/256 bits),
		);

// Mandatory Call Function: Call Order 2
void LEA_enc(
		LEA_st *lea
		);

// Mandatory Call Function: Call Order 2
void LEA_dec(
		LEA_st *lea
		);

#endif  //__LEA_H__
// End of lea.h
