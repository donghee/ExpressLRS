// ASCON-128a
//
// by Joungil Yun (sigipus@etri.re.kr)
//
// 2024.07.

#ifndef INC_ASCON128X_H_
#define INC_ASCON128X_H_

#include "ascon_aead_armv7m/ascon.h"
//#include "tools.h"

#define MAX_P_C_BYTE_LENGTH	256
#define MAX_A_BYTE_LENGTH	128

typedef struct
{
	uint8_t		K[16];						// Key
	uint32_t 	K_byte_length;				// Key byte length
	uint8_t 	T[16];						// Tag
	uint32_t 	T_byte_length;				// Tag byte length
	uint8_t 	N[16];						// N (public message, nonce)
	uint32_t 	N_byte_length;				// N byte length
	uint8_t 	A[MAX_A_BYTE_LENGTH];		// Associated Data
	uint32_t 	A_byte_length;				// A byte length

	uint8_t 	PP[MAX_P_C_BYTE_LENGTH];	// Plain(Decrypted) text data
	uint32_t 	PP_byte_length;				// PP byte length
	uint8_t 	CC[MAX_P_C_BYTE_LENGTH];	// Encrypted(Received) text data
	uint32_t 	CC_byte_length;				// CC byte length (should be the same length as PP_byte_length)
} ASCON_st;

// Reset CCM_st data structure. (clear all)
void ASCON128x_reset(
		ASCON_st *ascon
		);


// Optional Function
int ASCON128x_set_enc_params(
		ASCON_st *ascon,
		const uint8_t *PP,
		const uint32_t PP_byte_length,
		const uint8_t *N,
		const uint32_t N_byte_length
		);

// Optional Function
int ASCON128x_set_dec_params(
		ASCON_st *ascon,
		const uint8_t *CC,
		const uint32_t CC_byte_length,
		const uint8_t *N,
		const uint32_t N_byte_length,
		const uint8_t *T
		);

// Mandatory Function: Call Order 1
int ASCON128x_set_init_params(
		ASCON_st *ascon,
		const uint8_t *key,
		const uint32_t key_bit_length,
		const uint8_t *A,
		const uint32_t A_byte_length,
		const uint32_t T_bit_length
		);

// Mandatory Function: Call Order 2
int ASCON128x_enc(
		ASCON_st *ascon
		);

// Mandatory Function: Call Order 2
int ASCON128x_dec(
		ASCON_st *ascon
		);


#endif /* INC_ASCON128X_H_ */
// End of ascon128x.h
