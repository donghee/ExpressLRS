// Common Tools for LEA and mode of operations
//
// by Joungil Yun (sigipus@etri.re.kr)
//
// 2023.09.

#ifndef __TOOLS_H__
#define __TOOLS_H__

#include <stdio.h>
#include <string.h>
#include <stdint.h>


// For using the printf function for log output in the console during development.
#define _CONSOL_


#ifdef _CONSOL_
	#define CONSOL_PRINTF(...) { printf(__VA_ARGS__); }
	#define CONSOL_DEBUG(...)  { printf("[%s][Line %d]", &(__FUNCTION__), (__LINE__)); printf(__VA_ARGS__); }
#else
	#define CONSOL_PRINTF(...)
	#define CONSOL_DEBUG(...)
#endif


#define ROR32(_x, _n) (((uint32_t)(_x) >> (_n)) | ((uint32_t)(_x) << (32 - (_n))))
#define ROL32(_x, _n) (((uint32_t)(_x) << (_n)) | ((uint32_t)(_x) >> (32 - (_n))))

#define SHIFT32x4_R(_x, _n) { \
	*((uint32_t *)(_x) + 3) = (*((uint32_t *)(_x) + 3) >> (_n)) ^ (*((uint32_t *)(_x) + 2) << (32 - (_n))); \
	*((uint32_t *)(_x) + 2) = (*((uint32_t *)(_x) + 2) >> (_n)) ^ (*((uint32_t *)(_x) + 1) << (32 - (_n))); \
	*((uint32_t *)(_x) + 1) = (*((uint32_t *)(_x) + 1) >> (_n)) ^ (*((uint32_t *)(_x)    ) << (32 - (_n))); \
	*((uint32_t *)(_x)    ) =  *((uint32_t *)(_x)    ) >> (_n); \
	}

#define SHIFT32x4_L(_x, _n) { \
	*((uint32_t *)(_x)    ) = (*((uint32_t *)(_x)    ) << (_n)) ^ (*((uint32_t *)(_x) + 1) >> (32 - (_n))); \
	*((uint32_t *)(_x) + 1) = (*((uint32_t *)(_x) + 1) << (_n)) ^ (*((uint32_t *)(_x) + 2) >> (32 - (_n))); \
	*((uint32_t *)(_x) + 2) = (*((uint32_t *)(_x) + 2) << (_n)) ^ (*((uint32_t *)(_x) + 3) >> (32 - (_n))); \
	*((uint32_t *)(_x) + 3) =  *((uint32_t *)(_x) + 3) << (_n); \
	}

#define XOR32x4(_r, _a, _b) { \
	*((uint32_t *)(_r)    ) = *((uint32_t *)(_a)    ) ^ *((uint32_t *)(_b)    ); \
	*((uint32_t *)(_r) + 1) = *((uint32_t *)(_a) + 1) ^ *((uint32_t *)(_b) + 1); \
	*((uint32_t *)(_r) + 2) = *((uint32_t *)(_a) + 2) ^ *((uint32_t *)(_b) + 2); \
	*((uint32_t *)(_r) + 3) = *((uint32_t *)(_a) + 3) ^ *((uint32_t *)(_b) + 3); \
	}

#define SHIFT8x16_R8(_v)	{ \
	*((uint8_t *)(_v) + 15) = *((uint8_t *)(_v) + 14); \
	*((uint8_t *)(_v) + 14) = *((uint8_t *)(_v) + 13); \
	*((uint8_t *)(_v) + 13) = *((uint8_t *)(_v) + 12); \
	*((uint8_t *)(_v) + 12) = *((uint8_t *)(_v) + 11); \
	*((uint8_t *)(_v) + 11) = *((uint8_t *)(_v) + 10); \
	*((uint8_t *)(_v) + 10) = *((uint8_t *)(_v) +  9); \
	*((uint8_t *)(_v) +  9) = *((uint8_t *)(_v) +  8); \
	*((uint8_t *)(_v) +  8) = *((uint8_t *)(_v) +  7); \
	*((uint8_t *)(_v) +  7) = *((uint8_t *)(_v) +  6); \
	*((uint8_t *)(_v) +  6) = *((uint8_t *)(_v) +  5); \
	*((uint8_t *)(_v) +  5) = *((uint8_t *)(_v) +  4); \
	*((uint8_t *)(_v) +  4) = *((uint8_t *)(_v) +  3); \
	*((uint8_t *)(_v) +  3) = *((uint8_t *)(_v) +  2); \
	*((uint8_t *)(_v) +  2) = *((uint8_t *)(_v) +  1); \
	*((uint8_t *)(_v) +  1) = *((uint8_t *)(_v)     ); \
	*((uint8_t *)(_v)     ) = 0; \
	}

#define SHIFT8x16_R1(_v) { \
	*((uint8_t *)(_v) + 15) = (*((uint8_t *)(_v) + 15) >> 1) | (*((uint8_t *)(_v) + 14) << 7); \
	*((uint8_t *)(_v) + 14) = (*((uint8_t *)(_v) + 14) >> 1) | (*((uint8_t *)(_v) + 13) << 7); \
	*((uint8_t *)(_v) + 13) = (*((uint8_t *)(_v) + 13) >> 1) | (*((uint8_t *)(_v) + 12) << 7); \
	*((uint8_t *)(_v) + 12) = (*((uint8_t *)(_v) + 12) >> 1) | (*((uint8_t *)(_v) + 11) << 7); \
	*((uint8_t *)(_v) + 11) = (*((uint8_t *)(_v) + 11) >> 1) | (*((uint8_t *)(_v) + 10) << 7); \
	*((uint8_t *)(_v) + 10) = (*((uint8_t *)(_v) + 10) >> 1) | (*((uint8_t *)(_v) +  9) << 7); \
	*((uint8_t *)(_v) +  9) = (*((uint8_t *)(_v) +  9) >> 1) | (*((uint8_t *)(_v) +  8) << 7); \
	*((uint8_t *)(_v) +  8) = (*((uint8_t *)(_v) +  8) >> 1) | (*((uint8_t *)(_v) +  7) << 7); \
	*((uint8_t *)(_v) +  7) = (*((uint8_t *)(_v) +  7) >> 1) | (*((uint8_t *)(_v) +  6) << 7); \
	*((uint8_t *)(_v) +  6) = (*((uint8_t *)(_v) +  6) >> 1) | (*((uint8_t *)(_v) +  5) << 7); \
	*((uint8_t *)(_v) +  5) = (*((uint8_t *)(_v) +  5) >> 1) | (*((uint8_t *)(_v) +  4) << 7); \
	*((uint8_t *)(_v) +  4) = (*((uint8_t *)(_v) +  4) >> 1) | (*((uint8_t *)(_v) +  3) << 7); \
	*((uint8_t *)(_v) +  3) = (*((uint8_t *)(_v) +  3) >> 1) | (*((uint8_t *)(_v) +  2) << 7); \
	*((uint8_t *)(_v) +  2) = (*((uint8_t *)(_v) +  2) >> 1) | (*((uint8_t *)(_v) +  1) << 7); \
	*((uint8_t *)(_v) +  1) = (*((uint8_t *)(_v) +  1) >> 1) | (*((uint8_t *)(_v)     ) << 7); \
	*((uint8_t *)(_v)     ) = (*((uint8_t *)(_v)     ) >> 1); \
	}


uint32_t hexs2bytes(
		uint8_t *dst,
		const uint8_t *src,
		uint32_t hex_char_length
		);

void byte2word(
		uint32_t *dst,						// Big-Endian format word
		const uint8_t *src,
		const uint32_t src_byte_length		// Byte length of src pointer
		);

void word2byte(
		uint8_t *dst,
		const uint32_t *src,				// Big-Endian format word
		const uint32_t dst_byte_length);	// Byte length of dst pointer


void printf_8x16(uint8_t *x);
void printf_32x4(uint32_t *x);

#endif  //__TOOLS_H__
// End of tools.h
