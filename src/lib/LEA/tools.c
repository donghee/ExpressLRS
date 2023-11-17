// Common Tools for LEA and mode of operations
//
// by Joungil Yun (sigipus@etri.re.kr)
//
// 2023.09.

#include "tools.h"


// Converting a HEX code string into a byte array.
//
// input : uint8_t pointer (HEX string data)
// output: uint8_t pointer (byte data)
// return: byte length of the converted byte array
uint32_t hexs2bytes(uint8_t *output, const uint8_t *input, uint32_t hex_length)
{
	register uint8_t hex1;
	register uint8_t hex2;
	register uint32_t byte_length = hex_length >> 1;

	register int i;
	register int j;

	for(i = 0, j = 0; i < byte_length; ++i, j +=2)
	{
		hex1 = input[j    ];
		hex2 = input[j + 1];

		if(hex1 >= '0' && hex1 <= '9') hex1 = hex1 - '0';
		else if(hex1 >= 'A' && hex1 <= 'F') hex1 = hex1 - 'A' + 10;
		else if(hex1 >= 'a' && hex1 <= 'f') hex1 = hex1 - 'a' + 10;
		else return 0; // Invalid hexadecimal character input

		if(hex2 >= '0' && hex2 <= '9') hex2 = hex2 - '0';
		else if(hex2 >= 'A' && hex2 <= 'F') hex2 = hex2 - 'A' + 10;
		else if(hex2 >= 'a' && hex2 <= 'f') hex2 = hex2 - 'a' + 10;
		else return 0; // Invalid hexadecimal character input

		output[i] = (hex1 << 4) + hex2;
	}

	return byte_length;
}


// Word data byte packing.
typedef union {
	uint32_t 	v_32;
	uint8_t 	v_8[4];
} word_pack;

// Converting a byte array into a 32-bit word array in the Bit-Endian format.
//
// c.f. ARM Cortex-M4 is confirmed as Little-endian with the following code:
//      const int check = 1;
//	    if((*(char*)&check)==0) printf("Big-Endian\n");
//      else printf("Little-Endian\n");
//
// input : utin8_t pointer (source byte array), byte length of the input byte array
// output: uint32_t pointer (converted word array)
// return: None
//
// When 'src_byte_length' is not a multiple of 4, zero-padding is applied starting from the LSB.
void byte2word(uint32_t *dst, const uint8_t *src, const uint32_t src_byte_length)
{
	register int i = 0;
	register int j = 0;
	register uint32_t word_length = src_byte_length >> 2;
	register word_pack wp;

	for(; i < word_length; )
	{
		wp.v_8[3] = src[j    ];
		wp.v_8[2] = src[j + 1];
		wp.v_8[1] = src[j + 2];
		wp.v_8[0] = src[j + 3];
		dst[i] = wp.v_32;

		// for switch-case
		++i;
		j += 4;
	}

	switch(src_byte_length - j)
	{
	case 3:
		wp.v_8[3] = src[j    ];
		wp.v_8[2] = src[j + 1];
		wp.v_8[1] = src[j + 2];
		wp.v_8[0] = 0;
		dst[i] = wp.v_32;
		break;

	case 2:
		wp.v_8[3] = src[j    ];
		wp.v_8[2] = src[j + 1];
		wp.v_8[1] = 0;
		wp.v_8[0] = 0;
		dst[i] = wp.v_32;
		break;

	case 1:
		wp.v_8[3] = src[j    ];
		wp.v_8[2] = 0;
		wp.v_8[1] = 0;
		wp.v_8[0] = 0;
		dst[i] = wp.v_32;
		break;

	default:
		break;
	}
}

// Converting a 32-bit word array in the Big-Endian format into a byte array.
//
// input : utin32_t pointer (source word array), byte length of the output byte array
// output: uint8_t pointer (converted byte array)
// return: None
//
// The last word is partially converted into bytes according to 'dst_word_length' (from the MSB).
// The validity of 'src' and 'dst' memory access must be ensured according to 'dst_byte_length'.
void word2byte(uint8_t *dst, const uint32_t *src, const uint32_t dst_byte_length)
{
	register int i = 0;
	register int j = 0;
	register uint32_t word_length = dst_byte_length >> 2;
	register word_pack wp;

	for( ; i < word_length; )
	{
		wp.v_32 = src[i];
		dst[j    ] = wp.v_8[3];
		dst[j + 1] = wp.v_8[2];
		dst[j + 2] = wp.v_8[1];
		dst[j + 3] = wp.v_8[0];

		// for switch-case
		++i;
		j += 4;
	}

	switch(dst_byte_length - j)
	{
	case 3:
		wp.v_32 = src[i];
		dst[j    ] = wp.v_8[3];
		dst[j + 1] = wp.v_8[2];
		dst[j + 2] = wp.v_8[1];
		break;

	case 2:
		wp.v_32 = src[i];
		dst[j    ] = wp.v_8[3];
		dst[j + 1] = wp.v_8[2];
		break;

	case 1:
		wp.v_32 = src[i];
		dst[j    ] = wp.v_8[3];
		break;

	default:
		break;
	}

}


// just for debugging
void printf_8x16(uint8_t *x)
{
	register int i = 0;

	for(i = 0; i < 16; i++)
	{
		CONSOL_PRINTF("0x%02x ", x[i]);
	}
	CONSOL_PRINTF("\n");
}

// just for debugging
void printf_32x4(uint32_t *x)
{
	register int i = 0;

	for(i = 0; i < 4; i++)
	{
		CONSOL_PRINTF("0x%08x ", x[i]);
	}
	CONSOL_PRINTF("\n");
}

// End of tools.c
