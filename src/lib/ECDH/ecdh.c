// ECDH
//
// by Joungil Yun (sigipus@etri.re.kr)
//
// 2024.10.
//
// This code was developed with reference to micro-ecc, which is licensed under the BSD 2-clause license. (https://github.com/kmackay/micro-ecc)

#include "ecdh.h"

static ECC_RNG_Function g_rng_function = 0;

static void ECC_clear(uint32_t *vli, int8_t num_words)
{
	int8_t i;
	for (i = 0; i < num_words; ++i)
	{
		vli[i] = 0;
	}
}

static uint32_t ECC_isZero(const uint32_t *vli, int8_t num_words)
{
	uint32_t bits = 0;
	int8_t i;
	for (i = 0; i < num_words; ++i)
	{
		bits |= vli[i];
	}
	return (bits == 0);
}

static uint32_t ECC_testBit(const uint32_t *vli, int16_t bit)
{
	return (vli[bit >> 5] & ((uint32_t) 1 << (bit & 0x01F)));
}

static int8_t numDigits(const uint32_t *vli, const int8_t max_words)
{
	int8_t i;

	// Search from the end until we find a non-zero digit.
	// We do it in reverse because we expect that most digits will be nonzero.
	for (i = max_words - 1; i >= 0 && vli[i] == 0; --i)
	{
	}

	return (i + 1);
}

static int16_t ECC_numBits(const uint32_t *vli, const int8_t max_words)
{
	uint32_t i;
	uint32_t digit;

	int8_t num_digits = numDigits(vli, max_words);
	if (num_digits == 0)
	{
		return 0;
	}

	digit = vli[num_digits - 1];
	for (i = 0; digit; ++i)
	{
		digit >>= 1;
	}

	return (((int16_t) (num_digits - 1) << 5) + i);
}

static void ECC_set(uint32_t *dest, const uint32_t *src, int8_t num_words)
{
	int8_t i;
	for (i = 0; i < num_words; ++i)
	{
		dest[i] = src[i];
	}
}

static int8_t ECC_cmp_unsafe(const uint32_t *left, const uint32_t *right, int8_t num_words)
{
	int8_t i;
	for (i = num_words - 1; i >= 0; --i)
	{
		if (left[i] > right[i])
		{
			return 1;
		}
		else if (left[i] < right[i])
		{
			return -1;
		}
	}
	return 0;
}

static uint32_t ECC_equal(const uint32_t *left, const uint32_t *right, int8_t num_words)
{
	uint32_t diff = 0;
	int8_t i;
	for (i = num_words - 1; i >= 0; --i)
	{
		diff |= (left[i] ^ right[i]);
	}
	return (diff == 0);
}

static int8_t ECC_cmp(const uint32_t *left, const uint32_t *right, int8_t num_words)
{
	uint32_t tmp[ECC_MAX_WORDS];
	uint32_t neg = !!ECC_sub(tmp, left, right, num_words);
	uint32_t equal = ECC_isZero(tmp, num_words);
	return (!equal - 2 * neg);
}

static void ECC_rshift1(uint32_t *vli, int8_t num_words)
{
	uint32_t *end = vli;
	uint32_t carry = 0;

	vli += num_words;
	while (vli-- > end)
	{
		uint32_t temp = *vli;
		*vli = (temp >> 1) | carry;
		carry = temp << 31;
	}
}

static void ECC_modAdd(uint32_t *result, const uint32_t *left, const uint32_t *right, const uint32_t *mod, int8_t num_words)
{
	uint32_t carry = ECC_add(result, left, right, num_words);
	if (carry || ECC_cmp_unsafe(mod, result, num_words) != 1)
	{
		ECC_sub(result, result, mod, num_words);
	}
}

static void ECC_modSub(uint32_t *result, const uint32_t *left, const uint32_t *right, const uint32_t *mod, int8_t num_words)
{
	uint32_t l_borrow = ECC_sub(result, left, right, num_words);
	if (l_borrow)
	{
		ECC_add(result, result, mod, num_words);
	}
}

static void ECC_modMult_fast(uint32_t *result, const uint32_t *left, const uint32_t *right, ECC_Curve curve)
{
	uint32_t product[2 * ECC_MAX_WORDS];
	ECC_mult(product, left, right, curve->num_words);
	curve->mmod_fast(result, product);

}

static void ECC_modSquare_fast(uint32_t *result, const uint32_t *left, ECC_Curve curve)
{
	uint32_t product[2 * ECC_MAX_WORDS];
	ECC_square(product, left, curve->num_words);

	curve->mmod_fast(result, product);

}

#define EVEN(vli) (!(vli[0] & 1))
static void modInv_update(uint32_t *uv, const uint32_t *mod, int8_t num_words)
{
	uint32_t carry = 0;
	if (!EVEN(uv))
	{
		carry = ECC_add(uv, uv, mod, num_words);
	}
	ECC_rshift1(uv, num_words);
	if (carry)
	{
		uv[num_words - 1] |= 0x80000000;
	}
}

static void ECC_modInv(uint32_t *result, const uint32_t *input, const uint32_t *mod, int8_t num_words)
{
	uint32_t a[ECC_MAX_WORDS], b[ECC_MAX_WORDS], u[ECC_MAX_WORDS], v[ECC_MAX_WORDS];
	int8_t cmpResult;

	if (ECC_isZero(input, num_words))
	{
		ECC_clear(result, num_words);
		return;
	}

	ECC_set(a, input, num_words);
	ECC_set(b, mod, num_words);
	ECC_clear(u, num_words);
	u[0] = 1;
	ECC_clear(v, num_words);
	while ((cmpResult = ECC_cmp_unsafe(a, b, num_words)) != 0)
	{
		if (EVEN(a))
		{
			ECC_rshift1(a, num_words);
			modInv_update(u, mod, num_words);
		}
		else if (EVEN(b))
		{
			ECC_rshift1(b, num_words);
			modInv_update(v, mod, num_words);
		}
		else if (cmpResult > 0)
		{
			ECC_sub(a, a, b, num_words);
			ECC_rshift1(a, num_words);
			if (ECC_cmp_unsafe(u, v, num_words) < 0)
			{
				ECC_add(u, u, mod, num_words);
			}
			ECC_sub(u, u, v, num_words);
			modInv_update(u, mod, num_words);
		}
		else
		{
			ECC_sub(b, b, a, num_words);
			ECC_rshift1(b, num_words);
			if (ECC_cmp_unsafe(v, u, num_words) < 0)
			{
				ECC_add(v, v, mod, num_words);
			}
			ECC_sub(v, v, u, num_words);
			modInv_update(v, mod, num_words);
		}
	}
	ECC_set(result, u, num_words);
}

static void double_jacobian_default(uint32_t *X1, uint32_t *Y1, uint32_t *Z1, ECC_Curve curve);
static void x_side_default(uint32_t *result, const uint32_t *x, ECC_Curve curve);
static void mod_sqrt_default(uint32_t *a, ECC_Curve curve);
static void mmod_fast_secp256r1(uint32_t *result, uint32_t *product);

static const struct ECC_Curve_t curve_secp256r1 =
{
num_words_secp256r1, num_bytes_secp256r1, 256,
		{ BYTES_TO_WORDS_8(FF, FF, FF, FF, FF, FF, FF, FF), BYTES_TO_WORDS_8(FF, FF, FF, FF, 00, 00, 00, 00), BYTES_TO_WORDS_8(00, 00, 00, 00, 00, 00, 00, 00), BYTES_TO_WORDS_8(01, 00, 00, 00, FF, FF,
				FF, FF) },
		{ BYTES_TO_WORDS_8(51, 25, 63, FC, C2, CA, B9, F3), BYTES_TO_WORDS_8(84, 9E, 17, A7, AD, FA, E6, BC), BYTES_TO_WORDS_8(FF, FF, FF, FF, FF, FF, FF, FF), BYTES_TO_WORDS_8(00, 00, 00, 00, FF, FF,
				FF, FF) },
		{ BYTES_TO_WORDS_8(96, C2, 98, D8, 45, 39, A1, F4), BYTES_TO_WORDS_8(A0, 33, EB, 2D, 81, 7D, 03, 77), BYTES_TO_WORDS_8(F2, 40, A4, 63, E5, E6, BC, F8), BYTES_TO_WORDS_8(47, 42, 2C, E1, F2, D1,
				17, 6B),

		BYTES_TO_WORDS_8(F5, 51, BF, 37, 68, 40, B6, CB), BYTES_TO_WORDS_8(CE, 5E, 31, 6B, 57, 33, CE, 2B), BYTES_TO_WORDS_8(16, 9E, 0F, 7C, 4A, EB, E7, 8E), BYTES_TO_WORDS_8(9B, 7F, 1A, FE, E2, 42,
				E3, 4F) },
		{ BYTES_TO_WORDS_8(4B, 60, D2, 27, 3E, 3C, CE, 3B), BYTES_TO_WORDS_8(F6, B0, 53, CC, B0, 06, 1D, 65), BYTES_TO_WORDS_8(BC, 86, 98, 76, 55, BD, EB, B3), BYTES_TO_WORDS_8(E7, 93, 3A, AA, D8, 35,
				C6, 5A) }, &double_jacobian_default, &mod_sqrt_default, &x_side_default, &mmod_fast_secp256r1 };

ECC_Curve ECC_secp256r1(void)
{
	return &curve_secp256r1;
}

static void double_jacobian_default(uint32_t *X1, uint32_t *Y1, uint32_t *Z1, ECC_Curve curve)
{
	/* t1 = X, t2 = Y, t3 = Z */
	uint32_t t4[ECC_MAX_WORDS];
	uint32_t t5[ECC_MAX_WORDS];
	int8_t num_words = curve->num_words;

	if (ECC_isZero(Z1, num_words))
	{
		return;
	}

	ECC_modSquare_fast(t4, Y1, curve); 				// t4 = y1^2
	ECC_modMult_fast(t5, X1, t4, curve); 			// t5 = x1*y1^2 = A
	ECC_modSquare_fast(t4, t4, curve); 				// t4 = y1^4
	ECC_modMult_fast(Y1, Y1, Z1, curve); 			// t2 = y1*z1 = z3
	ECC_modSquare_fast(Z1, Z1, curve); 				// t3 = z1^2/

	ECC_modAdd(X1, X1, Z1, curve->p, num_words); 	// t1 = x1 + z1^2
	ECC_modAdd(Z1, Z1, Z1, curve->p, num_words); 	// t3 = 2*z1^2
	ECC_modSub(Z1, X1, Z1, curve->p, num_words); 	// t3 = x1 - z1^2
	ECC_modMult_fast(X1, X1, Z1, curve); 			// t1 = x1^2 - z1^4

	ECC_modAdd(Z1, X1, X1, curve->p, num_words); 	// t3 = 2*(x1^2 - z1^4)
	ECC_modAdd(X1, X1, Z1, curve->p, num_words); 	// t1 = 3*(x1^2 - z1^4)
	if (ECC_testBit(X1, 0))
	{
		uint32_t l_carry = ECC_add(X1, X1, curve->p, num_words);
		ECC_rshift1(X1, num_words);
		X1[num_words - 1] |= l_carry << 31;
	}
	else
	{
		ECC_rshift1(X1, num_words);
	}

	// t1 = 3/2*(x1^2 - z1^4) = B

	ECC_modSquare_fast(Z1, X1, curve); 				// t3 = B^2
	ECC_modSub(Z1, Z1, t5, curve->p, num_words); 	// t3 = B^2 - A
	ECC_modSub(Z1, Z1, t5, curve->p, num_words); 	// t3 = B^2 - 2A = x3
	ECC_modSub(t5, t5, Z1, curve->p, num_words); 	// t5 = A - x3
	ECC_modMult_fast(X1, X1, t5, curve);			// t1 = B * (A - x3)
	ECC_modSub(t4, X1, t4, curve->p, num_words);	// t4 = B * (A - x3) - y1^4 = y3

	ECC_set(X1, Z1, num_words);
	ECC_set(Z1, Y1, num_words);
	ECC_set(Y1, t4, num_words);
}

// Computes result = x^3 + ax + b. result must not overlap x.
static void x_side_default(uint32_t *result, const uint32_t *x, ECC_Curve curve)
{
	uint32_t _3[ECC_MAX_WORDS] =
	{ 3 }; 						// -a = 3
	int8_t num_words = curve->num_words;

	ECC_modSquare_fast(result, x, curve); 						// r = x^2
	ECC_modSub(result, result, _3, curve->p, num_words);		// r = x^2 - 3
	ECC_modMult_fast(result, result, x, curve);					// r = x^3 - 3x
	ECC_modAdd(result, result, curve->b, curve->p, num_words);	// r = x^3 - 3x + b
}

static void mod_sqrt_default(uint32_t *a, ECC_Curve curve)
{
	int16_t i;
	uint32_t p1[ECC_MAX_WORDS] =
	{ 1 };
	uint32_t l_result[ECC_MAX_WORDS] =
	{ 1 };
	int8_t num_words = curve->num_words;

	/* When curve->p == 3 (mod 4), we can compute
	 sqrt(a) = a^((curve->p + 1) / 4) (mod curve->p). */
	ECC_add(p1, curve->p, p1, num_words); /* p1 = curve_p + 1 */
	for (i = ECC_numBits(p1, num_words) - 1; i > 1; --i)
	{
		ECC_modSquare_fast(l_result, l_result, curve);
		if (ECC_testBit(p1, i))
		{
			ECC_modMult_fast(l_result, l_result, a, curve);
		}
	}
	ECC_set(a, l_result, num_words);
}

static void mmod_fast_secp256r1(uint32_t *result, uint32_t *product)
{
	uint32_t tmp[num_words_secp256r1];
	int carry;

	/* t */
	ECC_set(result, product, num_words_secp256r1);

	/* s1 */
	tmp[0] = tmp[1] = tmp[2] = 0;
	tmp[3] = product[11];
	tmp[4] = product[12];
	tmp[5] = product[13];
	tmp[6] = product[14];
	tmp[7] = product[15];
	carry = ECC_add(tmp, tmp, tmp, num_words_secp256r1);
	carry += ECC_add(result, result, tmp, num_words_secp256r1);

	/* s2 */
	tmp[3] = product[12];
	tmp[4] = product[13];
	tmp[5] = product[14];
	tmp[6] = product[15];
	tmp[7] = 0;
	carry += ECC_add(tmp, tmp, tmp, num_words_secp256r1);
	carry += ECC_add(result, result, tmp, num_words_secp256r1);

	/* s3 */
	tmp[0] = product[8];
	tmp[1] = product[9];
	tmp[2] = product[10];
	tmp[3] = tmp[4] = tmp[5] = 0;
	tmp[6] = product[14];
	tmp[7] = product[15];
	carry += ECC_add(result, result, tmp, num_words_secp256r1);

	/* s4 */
	tmp[0] = product[9];
	tmp[1] = product[10];
	tmp[2] = product[11];
	tmp[3] = product[13];
	tmp[4] = product[14];
	tmp[5] = product[15];
	tmp[6] = product[13];
	tmp[7] = product[8];
	carry += ECC_add(result, result, tmp, num_words_secp256r1);

	/* d1 */
	tmp[0] = product[11];
	tmp[1] = product[12];
	tmp[2] = product[13];
	tmp[3] = tmp[4] = tmp[5] = 0;
	tmp[6] = product[8];
	tmp[7] = product[10];
	carry -= ECC_sub(result, result, tmp, num_words_secp256r1);

	/* d2 */
	tmp[0] = product[12];
	tmp[1] = product[13];
	tmp[2] = product[14];
	tmp[3] = product[15];
	tmp[4] = tmp[5] = 0;
	tmp[6] = product[9];
	tmp[7] = product[11];
	carry -= ECC_sub(result, result, tmp, num_words_secp256r1);

	/* d3 */
	tmp[0] = product[13];
	tmp[1] = product[14];
	tmp[2] = product[15];
	tmp[3] = product[8];
	tmp[4] = product[9];
	tmp[5] = product[10];
	tmp[6] = 0;
	tmp[7] = product[12];
	carry -= ECC_sub(result, result, tmp, num_words_secp256r1);

	/* d4 */
	tmp[0] = product[14];
	tmp[1] = product[15];
	tmp[2] = 0;
	tmp[3] = product[9];
	tmp[4] = product[10];
	tmp[5] = product[11];
	tmp[6] = 0;
	tmp[7] = product[13];
	carry -= ECC_sub(result, result, tmp, num_words_secp256r1);

	if (carry < 0)
	{
		do
		{
			carry += ECC_add(result, result, curve_secp256r1.p,
			num_words_secp256r1);
		} while (carry < 0);
	}
	else
	{
		while (carry || ECC_cmp_unsafe(curve_secp256r1.p, result,
		num_words_secp256r1) != 1)
		{
			carry -= ECC_sub(result, result, curve_secp256r1.p,
			num_words_secp256r1);
		}
	}
}

static void double_jacobian_secp256k1(uint32_t *X1, uint32_t *Y1, uint32_t *Z1, ECC_Curve curve);
static void x_side_secp256k1(uint32_t *result, const uint32_t *x, ECC_Curve curve);
static void mmod_fast_secp256k1(uint32_t *result, uint32_t *product);

static const struct ECC_Curve_t curve_secp256k1 =
{
num_words_secp256k1, num_bytes_secp256k1, 256, /* num_n_bits */
		{ BYTES_TO_WORDS_8(2F, FC, FF, FF, FE, FF, FF, FF), BYTES_TO_WORDS_8(FF, FF, FF, FF, FF, FF, FF, FF), BYTES_TO_WORDS_8(FF, FF, FF, FF, FF, FF, FF, FF), BYTES_TO_WORDS_8(FF, FF, FF, FF, FF, FF,
				FF, FF) },
		{ BYTES_TO_WORDS_8(41, 41, 36, D0, 8C, 5E, D2, BF), BYTES_TO_WORDS_8(3B, A0, 48, AF, E6, DC, AE, BA), BYTES_TO_WORDS_8(FE, FF, FF, FF, FF, FF, FF, FF), BYTES_TO_WORDS_8(FF, FF, FF, FF, FF, FF,
				FF, FF) },
		{ BYTES_TO_WORDS_8(98, 17, F8, 16, 5B, 81, F2, 59), BYTES_TO_WORDS_8(D9, 28, CE, 2D, DB, FC, 9B, 02), BYTES_TO_WORDS_8(07, 0B, 87, CE, 95, 62, A0, 55), BYTES_TO_WORDS_8(AC, BB, DC, F9, 7E, 66,
				BE, 79),

		BYTES_TO_WORDS_8(B8, D4, 10, FB, 8F, D0, 47, 9C), BYTES_TO_WORDS_8(19, 54, 85, A6, 48, B4, 17, FD), BYTES_TO_WORDS_8(A8, 08, 11, 0E, FC, FB, A4, 5D), BYTES_TO_WORDS_8(65, C4, A3, 26, 77, DA,
				3A, 48) },
		{ BYTES_TO_WORDS_8(07, 00, 00, 00, 00, 00, 00, 00), BYTES_TO_WORDS_8(00, 00, 00, 00, 00, 00, 00, 00), BYTES_TO_WORDS_8(00, 00, 00, 00, 00, 00, 00, 00), BYTES_TO_WORDS_8(00, 00, 00, 00, 00, 00,
				00, 00) }, &double_jacobian_secp256k1, &mod_sqrt_default, &x_side_secp256k1, &mmod_fast_secp256k1 };

ECC_Curve ECC_secp256k1(void)
{
	return &curve_secp256k1;
}

static void double_jacobian_secp256k1(uint32_t *X1, uint32_t *Y1, uint32_t *Z1, ECC_Curve curve)
{
	/* t1 = X, t2 = Y, t3 = Z */
	uint32_t t4[num_words_secp256k1];
	uint32_t t5[num_words_secp256k1];

	if (ECC_isZero(Z1, num_words_secp256k1))
	{
		return;
	}

	ECC_modSquare_fast(t5, Y1, curve); /* t5 = y1^2 */
	ECC_modMult_fast(t4, X1, t5, curve); /* t4 = x1*y1^2 = A */
	ECC_modSquare_fast(X1, X1, curve); /* t1 = x1^2 */
	ECC_modSquare_fast(t5, t5, curve); /* t5 = y1^4 */
	ECC_modMult_fast(Z1, Y1, Z1, curve); /* t3 = y1*z1 = z3 */

	ECC_modAdd(Y1, X1, X1, curve->p, num_words_secp256k1); /* t2 = 2*x1^2 */
	ECC_modAdd(Y1, Y1, X1, curve->p, num_words_secp256k1); /* t2 = 3*x1^2 */
	if (ECC_testBit(Y1, 0))
	{
		uint32_t carry = ECC_add(Y1, Y1, curve->p, num_words_secp256k1);
		ECC_rshift1(Y1, num_words_secp256k1);
		Y1[num_words_secp256k1 - 1] |= carry << 31;
	}
	else
	{
		ECC_rshift1(Y1, num_words_secp256k1);
	}
	/* t2 = 3/2*(x1^2) = B */

	ECC_modSquare_fast(X1, Y1, curve); /* t1 = B^2 */
	ECC_modSub(X1, X1, t4, curve->p, num_words_secp256k1); /* t1 = B^2 - A */
	ECC_modSub(X1, X1, t4, curve->p, num_words_secp256k1); /* t1 = B^2 - 2A = x3 */

	ECC_modSub(t4, t4, X1, curve->p, num_words_secp256k1); /* t4 = A - x3 */
	ECC_modMult_fast(Y1, Y1, t4, curve); /* t2 = B * (A - x3) */
	ECC_modSub(Y1, Y1, t5, curve->p, num_words_secp256k1); /* t2 = B * (A - x3) - y1^4 = y3 */
}

/* Computes result = x^3 + b. result must not overlap x. */
static void x_side_secp256k1(uint32_t *result, const uint32_t *x, ECC_Curve curve)
{
	ECC_modSquare_fast(result, x, curve); /* r = x^2 */
	ECC_modMult_fast(result, result, x, curve); /* r = x^3 */
	ECC_modAdd(result, result, curve->b, curve->p, num_words_secp256k1); /* r = x^3 + b */
}

static void omega_mult_secp256k1(uint32_t *result, const uint32_t *right)
{
	/* Multiply by (2^9 + 2^8 + 2^7 + 2^6 + 2^4 + 1). */
	uint32_t carry = 0;
	int8_t k;

	for (k = 0; k < num_words_secp256k1; ++k)
	{
		uint64_t p = (uint64_t) 0x3D1 * right[k] + carry;
		result[k] = (uint32_t) p;
		carry = p >> 32;
	}
	result[num_words_secp256k1] = carry;
	/* add the 2^32 multiple */
	result[1 + num_words_secp256k1] = ECC_add(result + 1, result + 1, right,
	num_words_secp256k1);
}

static void mmod_fast_secp256k1(uint32_t *result, uint32_t *product)
{
	uint32_t tmp[2 * num_words_secp256k1];
	uint32_t carry;

	ECC_clear(tmp, num_words_secp256k1);
	ECC_clear(tmp + num_words_secp256k1, num_words_secp256k1);

	omega_mult_secp256k1(tmp, product + num_words_secp256k1); /* (Rq, q) = q * c */

	carry = ECC_add(result, product, tmp, num_words_secp256k1); /* (C, r) = r + q       */
	ECC_clear(product, num_words_secp256k1);
	omega_mult_secp256k1(product, tmp + num_words_secp256k1); /* Rq*c */
	carry += ECC_add(result, result, product, num_words_secp256k1); /* (C1, r) = r + Rq*c */

	while (carry > 0)
	{
		--carry;
		ECC_sub(result, result, curve_secp256k1.p, num_words_secp256k1);
	}
	if (ECC_cmp_unsafe(result, curve_secp256k1.p, num_words_secp256k1) > 0)
	{
		ECC_sub(result, result, curve_secp256k1.p, num_words_secp256k1);
	}
}

#define EccPoint_isZero(point, curve) ECC_isZero((point), (curve)->num_words * 2)

static void apply_z(uint32_t *X1, uint32_t *Y1, const uint32_t *const Z, ECC_Curve curve)
{
	uint32_t t1[ECC_MAX_WORDS];

	ECC_modSquare_fast(t1, Z, curve); /* z^2 */
	ECC_modMult_fast(X1, X1, t1, curve); /* x1 * z^2 */
	ECC_modMult_fast(t1, t1, Z, curve); /* z^3 */
	ECC_modMult_fast(Y1, Y1, t1, curve); /* y1 * z^3 */
}

static void XYcZ_initial_double(uint32_t *X1, uint32_t *Y1, uint32_t *X2, uint32_t *Y2, const uint32_t *const initial_Z, ECC_Curve curve)
{
	uint32_t z[ECC_MAX_WORDS];
	int8_t num_words = curve->num_words;
	if (initial_Z)
	{
		ECC_set(z, initial_Z, num_words);
	}
	else
	{
		ECC_clear(z, num_words);
		z[0] = 1;
	}

	ECC_set(X2, X1, num_words);
	ECC_set(Y2, Y1, num_words);

	apply_z(X1, Y1, z, curve);
	curve->double_jacobian(X1, Y1, z, curve);
	apply_z(X2, Y2, z, curve);
}

static void XYcZ_add(uint32_t *X1, uint32_t *Y1, uint32_t *X2, uint32_t *Y2, ECC_Curve curve)
{
	/* t1 = X1, t2 = Y1, t3 = X2, t4 = Y2 */
	uint32_t t5[ECC_MAX_WORDS];
	int8_t num_words = curve->num_words;

	ECC_modSub(t5, X2, X1, curve->p, num_words); /* t5 = x2 - x1 */
	ECC_modSquare_fast(t5, t5, curve); /* t5 = (x2 - x1)^2 = A */
	ECC_modMult_fast(X1, X1, t5, curve); /* t1 = x1*A = B */
	ECC_modMult_fast(X2, X2, t5, curve); /* t3 = x2*A = C */
	ECC_modSub(Y2, Y2, Y1, curve->p, num_words); /* t4 = y2 - y1 */
	ECC_modSquare_fast(t5, Y2, curve); /* t5 = (y2 - y1)^2 = D */

	ECC_modSub(t5, t5, X1, curve->p, num_words); /* t5 = D - B */
	ECC_modSub(t5, t5, X2, curve->p, num_words); /* t5 = D - B - C = x3 */
	ECC_modSub(X2, X2, X1, curve->p, num_words); /* t3 = C - B */
	ECC_modMult_fast(Y1, Y1, X2, curve); /* t2 = y1*(C - B) */
	ECC_modSub(X2, X1, t5, curve->p, num_words); /* t3 = B - x3 */
	ECC_modMult_fast(Y2, Y2, X2, curve); /* t4 = (y2 - y1)*(B - x3) */
	ECC_modSub(Y2, Y2, Y1, curve->p, num_words); /* t4 = y3 */

	ECC_set(X2, t5, num_words);
}

static void XYcZ_addC(uint32_t *X1, uint32_t *Y1, uint32_t *X2, uint32_t *Y2, ECC_Curve curve)
{
	/* t1 = X1, t2 = Y1, t3 = X2, t4 = Y2 */
	uint32_t t5[ECC_MAX_WORDS];
	uint32_t t6[ECC_MAX_WORDS];
	uint32_t t7[ECC_MAX_WORDS];
	int8_t num_words = curve->num_words;

	ECC_modSub(t5, X2, X1, curve->p, num_words); /* t5 = x2 - x1 */
	ECC_modSquare_fast(t5, t5, curve); /* t5 = (x2 - x1)^2 = A */
	ECC_modMult_fast(X1, X1, t5, curve); /* t1 = x1*A = B */
	ECC_modMult_fast(X2, X2, t5, curve); /* t3 = x2*A = C */
	ECC_modAdd(t5, Y2, Y1, curve->p, num_words); /* t5 = y2 + y1 */
	ECC_modSub(Y2, Y2, Y1, curve->p, num_words); /* t4 = y2 - y1 */

	ECC_modSub(t6, X2, X1, curve->p, num_words); /* t6 = C - B */
	ECC_modMult_fast(Y1, Y1, t6, curve); /* t2 = y1 * (C - B) = E */
	ECC_modAdd(t6, X1, X2, curve->p, num_words); /* t6 = B + C */
	ECC_modSquare_fast(X2, Y2, curve); /* t3 = (y2 - y1)^2 = D */
	ECC_modSub(X2, X2, t6, curve->p, num_words); /* t3 = D - (B + C) = x3 */

	ECC_modSub(t7, X1, X2, curve->p, num_words); /* t7 = B - x3 */
	ECC_modMult_fast(Y2, Y2, t7, curve); /* t4 = (y2 - y1)*(B - x3) */
	ECC_modSub(Y2, Y2, Y1, curve->p, num_words); /* t4 = (y2 - y1)*(B - x3) - E = y3 */

	ECC_modSquare_fast(t7, t5, curve); /* t7 = (y2 + y1)^2 = F */
	ECC_modSub(t7, t7, t6, curve->p, num_words); /* t7 = F - (B + C) = x3' */
	ECC_modSub(t6, t7, X1, curve->p, num_words); /* t6 = x3' - B */
	ECC_modMult_fast(t6, t6, t5, curve); /* t6 = (y2+y1)*(x3' - B) */
	ECC_modSub(Y1, t6, Y1, curve->p, num_words); /* t2 = (y2+y1)*(x3' - B) - E = y3' */

	ECC_set(X1, t7, num_words);
}

static void EccPoint_mult(uint32_t *result, const uint32_t *point, const uint32_t *scalar, const uint32_t *initial_Z, int16_t num_bits, ECC_Curve curve)
{
	/* R0 and R1 */
	uint32_t Rx[2][ECC_MAX_WORDS];
	uint32_t Ry[2][ECC_MAX_WORDS];
	uint32_t z[ECC_MAX_WORDS];
	int16_t i;
	uint32_t nb;
	int8_t num_words = curve->num_words;

	ECC_set(Rx[1], point, num_words);
	ECC_set(Ry[1], point + num_words, num_words);

	XYcZ_initial_double(Rx[1], Ry[1], Rx[0], Ry[0], initial_Z, curve);

	for (i = num_bits - 2; i > 0; --i)
	{
		nb = !ECC_testBit(scalar, i);
		XYcZ_addC(Rx[1 - nb], Ry[1 - nb], Rx[nb], Ry[nb], curve);
		XYcZ_add(Rx[nb], Ry[nb], Rx[1 - nb], Ry[1 - nb], curve);
	}

	nb = !ECC_testBit(scalar, 0);
	XYcZ_addC(Rx[1 - nb], Ry[1 - nb], Rx[nb], Ry[nb], curve);

	/* Find final 1/Z value. */
	ECC_modSub(z, Rx[1], Rx[0], curve->p, num_words); /* X1 - X0 */
	ECC_modMult_fast(z, z, Ry[1 - nb], curve); /* Yb * (X1 - X0) */
	ECC_modMult_fast(z, z, point, curve); /* xP * Yb * (X1 - X0) */
	ECC_modInv(z, z, curve->p, num_words); /* 1 / (xP * Yb * (X1 - X0)) */
	/* yP / (xP * Yb * (X1 - X0)) */
	ECC_modMult_fast(z, z, point + num_words, curve);
	ECC_modMult_fast(z, z, Rx[1 - nb], curve); /* Xb * yP / (xP * Yb * (X1 - X0)) */
	/* End 1/Z calculation */

	XYcZ_add(Rx[nb], Ry[nb], Rx[1 - nb], Ry[1 - nb], curve);
	apply_z(Rx[0], Ry[0], z, curve);

	ECC_set(result, Rx[0], num_words);
	ECC_set(result + num_words, Ry[0], num_words);
}

static uint32_t regularize_k(const uint32_t *const k, uint32_t *k0, uint32_t *k1, ECC_Curve curve)
{
	int8_t num_n_words = BITS_TO_WORDS(curve->num_n_bits);
	int16_t num_n_bits = curve->num_n_bits;
	uint32_t carry = ECC_add(k0, k, curve->n, num_n_words) || (num_n_bits < ((int16_t) num_n_words * 32) && ECC_testBit(k0, num_n_bits));
	ECC_add(k1, k0, curve->n, num_n_words);
	return carry;
}

static int ECC_generate_random_int(uint32_t *random, const uint32_t *top, int8_t num_words)
{
	uint32_t mask = (uint32_t) -1;
	uint32_t tries;
	int16_t num_bits = ECC_numBits(top, num_words);

	if (!g_rng_function)
	{
		return 0;
	}

	for (tries = 0; tries < ECC_RNG_MAX_TRIES; ++tries)
	{
		if (!g_rng_function((uint8_t*) random, num_words * 4))
		{
			return 0;
		}
		random[num_words - 1] &= mask >> ((int16_t) (num_words * 32 - num_bits));
		if (!ECC_isZero(random, num_words) && ECC_cmp(top, random, num_words) == 1)
		{
			return 1;
		}
	}
	return 0;
}

static uint32_t EccPoint_compute_public_key(uint32_t *result, uint32_t *private_key, ECC_Curve curve)
{
	uint32_t tmp1[ECC_MAX_WORDS];
	uint32_t tmp2[ECC_MAX_WORDS];
	uint32_t *p2[2] =
	{ tmp1, tmp2 };
	uint32_t *initial_Z = 0;
	uint32_t carry;

	carry = regularize_k(private_key, tmp1, tmp2, curve);

	if (g_rng_function)
	{
		if (!ECC_generate_random_int(p2[carry], curve->p, curve->num_words))
		{
			return 0;
		}
		initial_Z = p2[carry];
	}
	EccPoint_mult(result, curve->G, p2[!carry], initial_Z, curve->num_n_bits + 1, curve);

	if (EccPoint_isZero(result, curve))
	{
		return 0;
	}
	return 1;
}

static void ECC_nativeToBytes(uint8_t *bytes, int num_bytes, const uint32_t *native)
{
	int i;
	for (i = 0; i < num_bytes; ++i)
	{
		unsigned b = num_bytes - 1 - i;
		bytes[i] = native[b / 4] >> (8 * (b % 4));
	}
}

static void ECC_bytesToNative(uint32_t *native, const uint8_t *bytes, int num_bytes)
{
	int i;
	ECC_clear(native, (num_bytes + 3) / 4);
	for (i = 0; i < num_bytes; ++i)
	{
		unsigned b = num_bytes - 1 - i;
		native[b / 4] |= (uint32_t) bytes[i] << (8 * (b % 4));
	}
}

static int ECC_valid_point(const uint32_t *point, ECC_Curve curve)
{
	uint32_t tmp1[ECC_MAX_WORDS];
	uint32_t tmp2[ECC_MAX_WORDS];
	int8_t num_words = curve->num_words;

	if (EccPoint_isZero(point, curve))
	{
		return 0;
	}

	if (ECC_cmp_unsafe(curve->p, point, num_words) != 1 || ECC_cmp_unsafe(curve->p, point + num_words, num_words) != 1)
	{
		return 0;
	}

	ECC_modSquare_fast(tmp1, point + num_words, curve);
	curve->x_side(tmp2, point, curve); /* tmp2 = x^3 + ax + b */

	return (int) (ECC_equal(tmp1, tmp2, num_words));
}

int ECC_make_key(uint8_t *public_key, uint8_t *private_key, ECC_Curve curve)
{

	uint32_t _private[ECC_MAX_WORDS];
	uint32_t _public[ECC_MAX_WORDS * 2];

	uint32_t tries;

	for (tries = 0; tries < ECC_RNG_MAX_TRIES; ++tries)
	{
		if (!ECC_generate_random_int(_private, curve->n, BITS_TO_WORDS(curve->num_n_bits)))
		{
			return 0;
		}

		if (EccPoint_compute_public_key(_public, _private, curve))
		{

			ECC_nativeToBytes(private_key, BITS_TO_BYTES(curve->num_n_bits), _private);
			ECC_nativeToBytes(public_key, curve->num_bytes, _public);
			ECC_nativeToBytes(public_key + curve->num_bytes, curve->num_bytes, _public + curve->num_words);

			return 1;
		}
	}
	return 0;
}

int ECC_shared_secret(const uint8_t *public_key, const uint8_t *private_key, uint8_t *secret, ECC_Curve curve)
{
	uint32_t _public[ECC_MAX_WORDS * 2];
	uint32_t _private[ECC_MAX_WORDS];

	uint32_t tmp[ECC_MAX_WORDS];
	uint32_t *p2[2] =
	{ _private, tmp };
	uint32_t *initial_Z = 0;
	uint32_t carry;
	int8_t num_words = curve->num_words;
	int8_t num_bytes = curve->num_bytes;

	ECC_bytesToNative(_private, private_key, BITS_TO_BYTES(curve->num_n_bits));
	ECC_bytesToNative(_public, public_key, num_bytes);
	ECC_bytesToNative(_public + num_words, public_key + num_bytes, num_bytes);

	carry = regularize_k(_private, _private, tmp, curve);

	if (g_rng_function)
	{
		if (!ECC_generate_random_int(p2[carry], curve->p, num_words))
		{
			return 0;
		}
		initial_Z = p2[carry];
	}

	EccPoint_mult(_public, _public, p2[!carry], initial_Z, curve->num_n_bits + 1, curve);

	ECC_nativeToBytes(secret, num_bytes, _public);

	return !EccPoint_isZero(_public, curve);
}

void ECC_set_rng(ECC_RNG_Function rng_function)
{
	g_rng_function = rng_function;
}

ECC_RNG_Function ECC_get_rng(void)
{
	return g_rng_function;
}

int ECC_private_key_size(ECC_Curve curve)
{
	return BITS_TO_BYTES(curve->num_n_bits);
}

int ECC_public_key_size(ECC_Curve curve)
{
	return 2 * curve->num_bytes;
}

int ECC_valid_public_key(const uint8_t *public_key, ECC_Curve curve)
{

	uint32_t _public[ECC_MAX_WORDS * 2];

	ECC_bytesToNative(_public, public_key, curve->num_bytes);
	ECC_bytesToNative(_public + curve->num_words, public_key + curve->num_bytes, curve->num_bytes);

	return ECC_valid_point(_public, curve);
}

int ECC_compute_public_key(const uint8_t *private_key, uint8_t *public_key, ECC_Curve curve)
{

	uint32_t _private[ECC_MAX_WORDS];
	uint32_t _public[ECC_MAX_WORDS * 2];

	ECC_bytesToNative(_private, private_key, BITS_TO_BYTES(curve->num_n_bits));

	if (ECC_isZero(_private, BITS_TO_WORDS(curve->num_n_bits)))
	{
		return 0;
	}

	if (ECC_cmp(curve->n, _private, BITS_TO_WORDS(curve->num_n_bits)) != 1)
	{
		return 0;
	}

	if (!EccPoint_compute_public_key(_public, _private, curve))
	{
		return 0;
	}

	ECC_nativeToBytes(public_key, curve->num_bytes, _public);
	ECC_nativeToBytes(public_key + curve->num_bytes, curve->num_bytes, _public + curve->num_words);

	return 1;
}

void ECC_compress_public_key(const uint8_t *public_key, uint8_t *compressed, ECC_Curve curve)
{
	int i;
	for (i = 0; i < curve->num_bytes; ++i)
	{
		compressed[i + 1] = public_key[i];
	}

	compressed[0] = 2 + (public_key[curve->num_bytes * 2 - 1] & 0x01);
}

void ECC_decompress_public_key(const uint8_t *compressed, uint8_t *public_key, ECC_Curve curve)
{
	uint32_t point[ECC_MAX_WORDS * 2];
	uint32_t *y = point + curve->num_words;
	ECC_bytesToNative(point, compressed + 1, curve->num_bytes);

	curve->x_side(y, point, curve);
	curve->mod_sqrt(y, curve);

	if ((y[0] & 0x01) != (compressed[0] & 0x01))
	{
		ECC_sub(y, curve->p, y, curve->num_words);
	}

	ECC_nativeToBytes(public_key, curve->num_bytes, point);
	ECC_nativeToBytes(public_key + curve->num_bytes, curve->num_bytes, y);
}

// End of ecdh.c
