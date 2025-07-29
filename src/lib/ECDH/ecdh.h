// ECDH
//
// by Joungil Yun (sigipus@etri.re.kr)
//
// 2024.10.
//
// This code was developed with reference to micro-ecc, which is licensed under the BSD 2-clause license. (https://github.com/kmackay/micro-ecc)

#ifndef __ECDH_H__
#define __ECDH_H__

#include <stdint.h>

#define CONCATX(a, ...) a ## __VA_ARGS__
#define CONCAT(a, ...) CONCATX(a, __VA_ARGS__)

#define EVAL(...)  EVAL1(EVAL1(EVAL1(EVAL1(__VA_ARGS__))))
#define EVAL1(...) EVAL2(EVAL2(EVAL2(EVAL2(__VA_ARGS__))))
#define EVAL2(...) EVAL3(EVAL3(EVAL3(EVAL3(__VA_ARGS__))))
#define EVAL3(...) EVAL4(EVAL4(EVAL4(EVAL4(__VA_ARGS__))))
#define EVAL4(...) __VA_ARGS__

#define DEC_1  0
#define DEC_2  1
#define DEC_3  2
#define DEC_4  3
#define DEC_5  4
#define DEC_6  5
#define DEC_7  6
#define DEC_8  7
#define DEC_9  8
#define DEC_10 9
#define DEC_11 10
#define DEC_12 11
#define DEC_13 12
#define DEC_14 13
#define DEC_15 14
#define DEC_16 15
#define DEC_17 16
#define DEC_18 17
#define DEC_19 18
#define DEC_20 19
#define DEC_21 20
#define DEC_22 21
#define DEC_23 22
#define DEC_24 23
#define DEC_25 24
#define DEC_26 25
#define DEC_27 26
#define DEC_28 27
#define DEC_29 28
#define DEC_30 29
#define DEC_31 30
#define DEC_32 31

#define DEC(N) CONCAT(DEC_, N)

#define SECOND_ARG(_, val, ...) val
#define SOME_CHECK_0 ~, 0
#define GET_SECOND_ARG(...) SECOND_ARG(__VA_ARGS__, SOME,)
#define SOME_OR_0(N) GET_SECOND_ARG(CONCAT(SOME_CHECK_, N))

#define EMPTY(...)
#define DEFER(...) __VA_ARGS__ EMPTY()

#define REPEAT_NAME_0() REPEAT_0
#define REPEAT_NAME_SOME() REPEAT_SOME
#define REPEAT_0(...)
#define REPEAT_SOME(N, stuff) DEFER(CONCAT(REPEAT_NAME_, SOME_OR_0(DEC(N))))()(DEC(N), stuff) stuff
#define REPEAT(N, stuff) EVAL(REPEAT_SOME(N, stuff))

#define REPEATM_NAME_0() REPEATM_0
#define REPEATM_NAME_SOME() REPEATM_SOME
#define REPEATM_0(...)
#define REPEATM_SOME(N, macro) macro(N) \
	DEFER(CONCAT(REPEATM_NAME_, SOME_OR_0(DEC(N))))()(DEC(N), macro)
#define REPEATM(N, macro) EVAL(REPEATM_SOME(N, macro))

#define BITS_TO_WORDS(num_bits) ((num_bits + 31) / 32)
#define BITS_TO_BYTES(num_bits) ((num_bits + 7) / 8)

#define REG_RW "+&r"
#define REG_WRITE "=&r"
#define REG_RW_LO "+&r"
#define REG_WRITE_LO "=&r"
#define RESUME_SYNTAX ".syntax divided \n\t"

struct ECC_Curve_t;
typedef const struct ECC_Curve_t *ECC_Curve;

#define ECC_RNG_MAX_TRIES 64

#define ECC_MAX_WORDS 8 // fixed

#define num_bytes_secp256r1 32
#define num_bytes_secp256k1 32

#define num_words_secp256r1 8
#define num_words_secp256k1 8

#define BYTES_TO_WORDS_8(a, b, c, d, e, f, g, h) 0x##d##c##b##a, 0x##h##g##f##e
#define BYTES_TO_WORDS_4(a, b, c, d) 0x##d##c##b##a



static uint32_t ECC_add(uint32_t *result, const uint32_t *left, const uint32_t *right, int8_t num_words)
{
	uint32_t carry;
	uint32_t left_word;
	uint32_t right_word;

	__asm__ volatile (
			".syntax unified \n\t"
			"movs %[carry], #0 \n\t"
			"ldmia %[lptr]!, {%[left]} \n\t"
			"ldmia %[rptr]!, {%[right]} \n\t"
			"adds %[left], %[right] \n\t"
			"stmia %[dptr]!, {%[left]} \n\t"
			"1: \n\t"
			REPEAT(DEC(ECC_MAX_WORDS),
					"ldmia %[lptr]!, {%[left]} \n\t"
					"ldmia %[rptr]!, {%[right]} \n\t"
					"adcs %[left], %[right] \n\t"
					"stmia %[dptr]!, {%[left]} \n\t")
			"adcs %[carry], %[carry] \n\t"
			RESUME_SYNTAX
			: [dptr] REG_RW_LO (result), [lptr] REG_RW_LO (left), [rptr] REG_RW_LO (right),
			[carry] REG_WRITE_LO (carry), [left] REG_WRITE_LO (left_word),
			[right] REG_WRITE_LO (right_word)
			:
			: "cc", "memory"
	);
	return carry;
}

static uint32_t ECC_sub(uint32_t *result, const uint32_t *left, const uint32_t *right, int8_t num_words)
{
	uint32_t carry;
	uint32_t left_word;
	uint32_t right_word;

	__asm__ volatile (
			".syntax unified \n\t"
			"movs %[carry], #0 \n\t"
			"ldmia %[lptr]!, {%[left]} \n\t"
			"ldmia %[rptr]!, {%[right]} \n\t"
			"subs %[left], %[right] \n\t"
			"stmia %[dptr]!, {%[left]} \n\t"
			"1: \n\t"
			REPEAT(DEC(ECC_MAX_WORDS),
					"ldmia %[lptr]!, {%[left]} \n\t"
					"ldmia %[rptr]!, {%[right]} \n\t"
					"sbcs %[left], %[right] \n\t"
					"stmia %[dptr]!, {%[left]} \n\t")
			"adcs %[carry], %[carry] \n\t"
			RESUME_SYNTAX
			: [dptr] REG_RW_LO (result), [lptr] REG_RW_LO (left), [rptr] REG_RW_LO (right),
			[carry] REG_WRITE_LO (carry), [left] REG_WRITE_LO (left_word),
			[right] REG_WRITE_LO (right_word)
			:
			: "cc", "memory"
	);
	return !carry; /* Note that on ARM, carry flag set means "no borrow" when subtracting
	 (for some reason...) */
}

static void ECC_mult(uint32_t *result, const uint32_t *left, const uint32_t *right, int8_t num_words)
{
	register uint32_t *r0 __asm__("r0") = result;
	register const uint32_t *r1 __asm__("r1") = left;
	register const uint32_t *r2 __asm__("r2") = right;
	register uint32_t r3 __asm__("r3") = num_words;

	__asm__ volatile (
			".syntax unified \n\t"
			"ldmia  r2!, {r4, r5, r6, r7} \n\t"
			"ldr    r14, [r1], #4 \n\t"
			"umull  r8, r9, r4, r14 \n\t"
			"str    r8, [r0], #4 \n\t"
			"mov    r10, #0 \n\t"
			"umaal  r9, r10, r5, r14 \n\t"
			"mov    r11, #0 \n\t"
			"umaal  r10, r11, r6, r14 \n\t"
			"mov    r12, #0 \n\t"
			"umaal  r11, r12, r7, r14 \n\t"
			"ldr    r14, [r1], #4 \n\t"
			"mov    r8, #0 \n\t"
			"umaal  r8, r9, r4, r14 \n\t"
			"str    r8, [r0], #4 \n\t"
			"umaal  r9, r10, r5, r14 \n\t"
			"umaal  r10, r11, r6, r14 \n\t"
			"umaal  r11, r12, r7, r14 \n\t"
			"ldr    r14, [r1], #4 \n\t"
			"mov    r8, #0 \n\t"
			"umaal  r8, r9, r4, r14 \n\t"
			"str    r8, [r0], #4 \n\t"
			"umaal  r9, r10, r5, r14 \n\t"
			"umaal  r10, r11, r6, r14 \n\t"
			"umaal  r11, r12, r7, r14 \n\t"
			"ldr    r14, [r1], #4 \n\t"
			"mov    r8, #0 \n\t"
			"umaal  r8, r9, r4, r14 \n\t"
			"str    r8, [r0], #4 \n\t"
			"umaal  r9, r10, r5, r14 \n\t"
			"umaal  r10, r11, r6, r14 \n\t"
			"umaal  r11, r12, r7, r14 \n\t"
			"ldr    r14, [r1], #4 \n\t"
			"mov    r8, #0 \n\t"
			"umaal  r8, r9, r4, r14 \n\t"
			"str    r8, [r0], #4 \n\t"
			"umaal  r9, r10, r5, r14 \n\t"
			"umaal  r10, r11, r6, r14 \n\t"
			"umaal  r11, r12, r7, r14 \n\t"
			"ldr    r14, [r1], #4 \n\t"
			"mov    r8, #0 \n\t"
			"umaal  r8, r9, r4, r14 \n\t"
			"str    r8, [r0], #4 \n\t"
			"umaal  r9, r10, r5, r14 \n\t"
			"umaal  r10, r11, r6, r14 \n\t"
			"umaal  r11, r12, r7, r14 \n\t"
			"ldr    r14, [r1], #4 \n\t"
			"mov    r8, #0 \n\t"
			"umaal  r8, r9, r4, r14 \n\t"
			"str    r8, [r0], #4 \n\t"
			"umaal  r9, r10, r5, r14 \n\t"
			"umaal  r10, r11, r6, r14 \n\t"
			"umaal  r11, r12, r7, r14 \n\t"
			"ldr    r14, [r1], #4 \n\t"
			"mov    r8, #0 \n\t"
			"umaal  r8, r9, r4, r14 \n\t"
			"str    r8, [r0], #4 \n\t"
			"umaal  r9, r10, r5, r14 \n\t"
			"umaal  r10, r11, r6, r14 \n\t"
			"umaal  r11, r12, r7, r14 \n\t"
			"str    r9, [r0], #4 \n\t"
			"str    r10, [r0], #4 \n\t"
			"str    r11, [r0], #4 \n\t"
			"str    r12, [r0], #4 \n\t"
			"sub r0, #32 \n\t"
			"sub r1, #32 \n\t"
			"ldmia  r2!, {r4, r5, r6, r7} \n\t"
			"ldr    r14, [r1], #4 \n\t"
			"ldr    r8, [r0] \n\t"
			"mov    r9, #0 \n\t"
			"umaal  r8, r9, r4, r14 \n\t"
			"str    r8, [r0], #4 \n\t"
			"mov    r10, #0 \n\t"
			"umaal  r9, r10, r5, r14 \n\t"
			"mov    r11, #0 \n\t"
			"umaal  r10, r11, r6, r14 \n\t"
			"mov    r12, #0 \n\t"
			"umaal  r11, r12, r7, r14 \n\t"
			"ldr    r14, [r1], #4 \n\t"
			"ldr    r8, [r0] \n\t"
			"umaal  r8, r9, r4, r14 \n\t"
			"str    r8, [r0], #4 \n\t"
			"umaal  r9, r10, r5, r14 \n\t"
			"umaal  r10, r11, r6, r14 \n\t"
			"umaal  r11, r12, r7, r14 \n\t"
			"ldr    r14, [r1], #4 \n\t"
			"ldr    r8, [r0] \n\t"
			"umaal  r8, r9, r4, r14 \n\t"
			"str    r8, [r0], #4 \n\t"
			"umaal  r9, r10, r5, r14 \n\t"
			"umaal  r10, r11, r6, r14 \n\t"
			"umaal  r11, r12, r7, r14 \n\t"
			"ldr    r14, [r1], #4 \n\t"
			"ldr    r8, [r0] \n\t"
			"umaal  r8, r9, r4, r14 \n\t"
			"str    r8, [r0], #4 \n\t"
			"umaal  r9, r10, r5, r14 \n\t"
			"umaal  r10, r11, r6, r14 \n\t"
			"umaal  r11, r12, r7, r14 \n\t"
			"ldr    r14, [r1], #4 \n\t"
			"ldr    r8, [r0] \n\t"
			"umaal  r8, r9, r4, r14 \n\t"
			"str    r8, [r0], #4 \n\t"
			"umaal  r9, r10, r5, r14 \n\t"
			"umaal  r10, r11, r6, r14 \n\t"
			"umaal  r11, r12, r7, r14 \n\t"
			"ldr    r14, [r1], #4 \n\t"
			"ldr    r8, [r0] \n\t"
			"umaal  r8, r9, r4, r14 \n\t"
			"str    r8, [r0], #4 \n\t"
			"umaal  r9, r10, r5, r14 \n\t"
			"umaal  r10, r11, r6, r14 \n\t"
			"umaal  r11, r12, r7, r14 \n\t"
			"ldr    r14, [r1], #4 \n\t"
			"ldr    r8, [r0] \n\t"
			"umaal  r8, r9, r4, r14 \n\t"
			"str    r8, [r0], #4 \n\t"
			"umaal  r9, r10, r5, r14 \n\t"
			"umaal  r10, r11, r6, r14 \n\t"
			"umaal  r11, r12, r7, r14 \n\t"
			"ldr    r14, [r1], #4 \n\t"
			"ldr    r8, [r0] \n\t"
			"umaal  r8, r9, r4, r14 \n\t"
			"str    r8, [r0], #4 \n\t"
			"umaal  r9, r10, r5, r14 \n\t"
			"umaal  r10, r11, r6, r14 \n\t"
			"umaal  r11, r12, r7, r14 \n\t"
			"str    r9, [r0], #4 \n\t"
			"str    r10, [r0], #4 \n\t"
			"str    r11, [r0], #4 \n\t"
			"str    r12, [r0], #4 \n\t"
			"1: \n\t"
			RESUME_SYNTAX
			: "+r" (r0), "+r" (r1), "+r" (r2)
			: "r" (r3)
			: "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "r14", "cc", "memory"
	);
}

static void ECC_square(uint32_t *result, const uint32_t *left, int8_t num_words)
{
	register uint32_t *r0 __asm__("r0") = result;
	register const uint32_t *r1 __asm__("r1") = left;
	register uint32_t r2 __asm__("r2") = num_words;

	__asm__ volatile (
			".syntax unified \n\t"
			"ldmia r1!, {r10,r11,r12,r14} \n\t"
			"push {r2} \n\t"
			"umull r2, r3, r11, r10 \n\t"
			"mov r4, #0 \n\t"
			"umaal r3, r4, r12, r10 \n\t"
			"mov r5, #0 \n\t"
			"umaal r4, r5, r14, r10 \n\t"
			"mov r6, #0 \n\t"
			"umaal r6, r4, r12, r11 \n\t"
			"adds r2, r2, r2 \n\t"
			"adcs r3, r3, r3 \n\t"
			"adcs r6, r6, r6 \n\t"
			"mov r7, #0 \n\t"
			"adc r7, r7, #0 \n\t"
			"umull r8, r9, r10, r10 \n\t"
			"adds r9, r9, r2 \n\t"
			"stmia r0!, {r8,r9} \n\t"
			"umull r8, r9, r11, r11 \n\t"
			"adcs r8, r8, r3 \n\t"
			"adcs r9, r9, r6 \n\t"
			"stmia r0!, {r8,r9} \n\t"
			"mov r8, #0 \n\t"
			"adc r8, r8, #0 \n\t"
			"ldmia r1!, {r2, r3} \n\t"
			"push {r1} \n\t"
			"umaal r4, r5, r2, r10 \n\t"
			"mov r6, #0 \n\t"
			"umaal r5, r6, r3, r10 \n\t"
			"mov r9, #0 \n\t"
			"umaal r9, r4, r14, r11 \n\t"
			"umaal r4, r5, r2, r11 \n\t"
			"mov r1, #0 \n\t"
			"umaal r1, r4, r14, r12 \n\t"
			"lsrs r7, #1 \n\t"
			"adcs r9, r9, r9 \n\t"
			"adcs r1, r1, r1 \n\t"
			"adc r7, r7, #0 \n\t"
			"umaal r8, r9, r12, r12 \n\t"
			"adds r9, r9, r1 \n\t"
			"stmia r0!, {r8,r9} \n\t"
			"mov r8, #0 \n\t"
			"adc r8, r8, #0 \n\t"
			"pop {r1} \n\t"
			"ldr r9, [r1], #4 \n\t"
			"ldr r1, [r1] \n\t"
			"push {r7} \n\t"
			"umaal r5, r6, r9, r10 \n\t"
			"mov r7, #0 \n\t"
			"umaal r6, r7, r1, r10 \n\t"
			"pop {r10} \n\t"
			"umaal r4, r5, r3, r11 \n\t"
			"umaal r5, r6, r9, r11 \n\t"
			"umaal r6, r7, r1, r11 \n\t"
			"mov r11, #0 \n\t"
			"umaal r11, r4, r2, r12 \n\t"
			"umaal r4, r5, r3, r12 \n\t"
			"umaal r5, r6, r9, r12 \n\t"
			"umaal r6, r7, r1, r12 \n\t"
			"mov r12, #0 \n\t"
			"umaal r12, r4, r2, r14 \n\t"
			"umaal r4, r5, r3, r14 \n\t"
			"umaal r5, r6, r9, r14 \n\t"
			"umaal r6, r7, r1, r14 \n\t"
			"lsrs r10, #1 \n\t"
			"adcs r11, r11, r11 \n\t"
			"adcs r12, r12, r12 \n\t"
			"adc r10, r10, #0 \n\t"
			"umaal r8, r11, r14, r14 \n\t"
			"adds r11, r11, r12 \n\t"
			"stmia r0!, {r8,r11} \n\t"
			"mov r8, #0 \n\t"
			"adc r8, r8, #0 \n\t"
			"mov r11, #0 \n\t"
			"umaal r11, r5, r3, r2 \n\t"
			"umaal r5, r6, r9, r2 \n\t"
			"umaal r6, r7, r1, r2 \n\t"
			"mov r12, #0 \n\t"
			"umaal r12, r6, r9, r3 \n\t"
			"umaal r6, r7, r1, r3 \n\t"
			"mov r14, #0 \n\t"
			"umaal r14, r7, r1, r9 \n\t"
			"lsrs r10, #1 \n\t"
			"adcs r4, r4, r4 \n\t"
			"adcs r11, r11, r11 \n\t"
			"adcs r5, r5, r5 \n\t"
			"adcs r12, r12, r12 \n\t"
			"adcs r6, r6, r6 \n\t"
			"adcs r14, r14, r14 \n\t"
			"adcs r7, r7, r7 \n\t"
			"adc r10, r10, #0 \n\t"
			"umaal r4, r8, r2, r2 \n\t"
			"adds r8, r8, r11 \n\t"
			"stmia r0!, {r4,r8} \n\t"
			"umull r4, r8, r3, r3 \n\t"
			"adcs r4, r4, r5 \n\t"
			"adcs r8, r8, r12 \n\t"
			"stmia r0!, {r4,r8} \n\t"
			"umull r4, r8, r9, r9 \n\t"
			"adcs r4, r4, r6 \n\t"
			"adcs r8, r8, r14 \n\t"
			"stmia r0!, {r4,r8} \n\t"
			"umull r4, r8, r1, r1 \n\t"
			"adcs r4, r4, r7 \n\t"
			"adcs r8, r8, r10 \n\t"
			"stmia r0!, {r4,r8} \n\t"
			"pop {r2} \n\t"
			"1: \n\t"
			RESUME_SYNTAX
			: "+r" (r0), "+r" (r1)
			: "r" (r2)
			: "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "r14", "cc", "memory"
	);
}





struct ECC_Curve_t
{
	int8_t num_words;
	int8_t num_bytes;
	int16_t num_n_bits;
	uint32_t p[ECC_MAX_WORDS];
	uint32_t n[ECC_MAX_WORDS];
	uint32_t G[ECC_MAX_WORDS * 2];
	uint32_t b[ECC_MAX_WORDS];
	void (*double_jacobian)(uint32_t *X1, uint32_t *Y1, uint32_t *Z1, ECC_Curve curve);
	void (*mod_sqrt)(uint32_t *a, ECC_Curve curve);
	void (*x_side)(uint32_t *result, const uint32_t *x, ECC_Curve curve);
	void (*mmod_fast)(uint32_t *result, uint32_t *product);
};

typedef int (*ECC_RNG_Function)(uint8_t *dest, unsigned size);

void ECC_set_rng(ECC_RNG_Function rng_function);
ECC_RNG_Function ECC_get_rng(void);

ECC_Curve ECC_secp256r1(void);
ECC_Curve ECC_secp256k1(void);

int ECC_private_key_size(ECC_Curve curve);
int ECC_public_key_size(ECC_Curve curve);

int ECC_make_key(uint8_t *public_key, uint8_t *private_key, ECC_Curve curve);
int ECC_valid_public_key(const uint8_t *public_key, ECC_Curve curve);
int ECC_compute_public_key(const uint8_t *private_key, uint8_t *public_key, ECC_Curve curve);

int ECC_shared_secret(const uint8_t *public_key, const uint8_t *private_key, uint8_t *secret, ECC_Curve curve);

void ECC_compress_public_key(const uint8_t *public_key, uint8_t *compressed, ECC_Curve curve);
void ECC_decompress_public_key(const uint8_t *compressed, uint8_t *public_key, ECC_Curve curve);

#endif  // __ECDH_H__
//End of ecdh.h
