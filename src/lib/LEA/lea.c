// LEA
//   - Round Key Scheduler
//   - Encryption
//   - Decryption
//
// by Joungil Yun (sigipus@etri.re.kr)
//
// 2023.09.

#include "lea.h"


// from NSR & KISA LEA reference code
static uint32_t delta[8][36] = {
			{0xc3efe9db, 0x87dfd3b7, 0x0fbfa76f, 0x1f7f4ede, 0x3efe9dbc, 0x7dfd3b78, 0xfbfa76f0, 0xf7f4ede1,
			0xefe9dbc3, 0xdfd3b787, 0xbfa76f0f, 0x7f4ede1f, 0xfe9dbc3e, 0xfd3b787d, 0xfa76f0fb, 0xf4ede1f7,
			0xe9dbc3ef, 0xd3b787df, 0xa76f0fbf, 0x4ede1f7f, 0x9dbc3efe, 0x3b787dfd, 0x76f0fbfa, 0xede1f7f4,
			0xdbc3efe9, 0xb787dfd3, 0x6f0fbfa7, 0xde1f7f4e, 0xbc3efe9d, 0x787dfd3b, 0xf0fbfa76, 0xe1f7f4eD,
			0xc3efe9db,	0x87dfd3b7, 0x0fbfa76f, 0x1f7f4ede},
			{0x44626b02, 0x88c4d604, 0x1189ac09, 0x23135812, 0x4626b024, 0x8c4d6048, 0x189ac091, 0x31358122,
			0x626b0244, 0xc4d60488, 0x89ac0911, 0x13581223, 0x26b02446, 0x4d60488c, 0x9ac09118, 0x35812231,
			0x6b024462, 0xd60488c4, 0xac091189, 0x58122313, 0xb0244626, 0x60488c4d, 0xc091189a, 0x81223135,
			0x0244626b, 0x0488c4d6, 0x091189ac, 0x12231358, 0x244626b0, 0x488c4d60, 0x91189ac0, 0x22313581,
			0x44626b02, 0x88c4d604, 0x1189ac09, 0x23135812},
			{0x79e27c8a, 0xf3c4f914, 0xe789f229, 0xcf13e453, 0x9e27c8a7, 0x3c4f914f, 0x789f229e, 0xf13e453c,
			0xe27c8a79, 0xc4f914f3, 0x89f229e7, 0x13e453cf, 0x27c8a79e, 0x4f914f3c, 0x9f229e78, 0x3e453cf1,
			0x7c8a79e2, 0xf914f3c4, 0xf229e789, 0xe453cf13, 0xc8a79e27, 0x914f3c4f, 0x229e789f, 0x453cf13e,
			0x8a79e27c, 0x14f3c4f9, 0x29e789f2, 0x53cf13e4, 0xa79e27c8, 0x4f3c4f91, 0x9e789f22, 0x3cf13e45,
			0x79e27c8a, 0xf3c4f914, 0xe789f229, 0xcf13e453},
			{0x78df30ec, 0xf1be61d8, 0xe37cc3b1, 0xc6f98763, 0x8df30ec7, 0x1be61d8f, 0x37cc3b1e, 0x6f98763c,
			0xdf30ec78, 0xbe61d8f1, 0x7cc3b1e3, 0xf98763c6, 0xf30ec78d, 0xe61d8f1b, 0xcc3b1e37, 0x98763c6f,
			0x30ec78df, 0x61d8f1be, 0xc3b1e37c, 0x8763c6f9, 0x0ec78df3, 0x1d8f1be6, 0x3b1e37cc, 0x763c6f98,
			0xec78df30, 0xd8f1be61, 0xb1e37cc3, 0x63c6f987, 0xc78df30e, 0x8f1be61d, 0x1e37cc3b, 0x3c6f9876,
			0x78df30ec,	0xf1be61d8, 0xe37cc3b1, 0xc6f98763},
			{0x715ea49e, 0xe2bd493c, 0xc57a9279, 0x8af524f3, 0x15ea49e7, 0x2bd493ce, 0x57a9279c, 0xaf524f38,
			0x5ea49e71, 0xbd493ce2, 0x7a9279c5, 0xf524f38a, 0xea49e715, 0xd493ce2b, 0xa9279c57, 0x524f38af,
			0xa49e715e, 0x493ce2bd, 0x9279c57a, 0x24f38af5, 0x49e715ea, 0x93ce2bd4, 0x279c57a9, 0x4f38af52,
			0x9e715ea4, 0x3ce2bd49, 0x79c57a92, 0xf38af524, 0xe715ea49, 0xce2bd493, 0x9c57a927, 0x38af524f,
			0x715ea49e,	0xe2bd493c, 0xc57a9279, 0x8af524f3},
			{0xc785da0a, 0x8f0bb415, 0x1e17682b, 0x3c2ed056, 0x785da0ac, 0xf0bb4158, 0xe17682b1, 0xc2ed0563,
			0x85da0ac7, 0x0bb4158f, 0x17682b1e, 0x2ed0563c, 0x5da0ac78, 0xbb4158f0, 0x7682b1e1, 0xed0563c2,
			0xda0ac785, 0xb4158f0b, 0x682b1e17, 0xd0563c2e, 0xa0ac785d, 0x4158f0bb, 0x82b1e176, 0x0563c2ed,
			0x0ac785da, 0x158f0bb4, 0x2b1e1768, 0x563c2ed0, 0xac785da0, 0x58f0bb41, 0xb1e17682, 0x63c2ed05,
			0xc785da0a, 0x8f0bb415, 0x1e17682b, 0x3c2ed056},
			{0xe04ef22a, 0xc09de455, 0x813bc8ab, 0x02779157, 0x04ef22ae, 0x09de455c, 0x13bc8ab8, 0x27791570,
			0x4ef22ae0, 0x9de455c0, 0x3bc8ab81, 0x77915702, 0xef22ae04, 0xde455c09, 0xbc8ab813, 0x79157027,
			0xf22ae04e, 0xe455c09d, 0xc8ab813b, 0x91570277, 0x22ae04ef, 0x455c09de, 0x8ab813bc, 0x15702779,
			0x2ae04ef2, 0x55c09de4, 0xab813bc8, 0x57027791, 0xae04ef22, 0x5c09de45, 0xb813bc8a, 0x70277915,
			0xe04ef22a,	0xc09de455, 0x813bc8ab, 0x02779157},
			{0xe5c40957, 0xcb8812af, 0x9710255f, 0x2e204abf, 0x5c40957e, 0xb8812afc, 0x710255f9, 0xe204abf2,
			0xc40957e5, 0x8812afcb, 0x10255f97, 0x204abf2e, 0x40957e5c, 0x812afcb8, 0x0255f971, 0x04abf2e2,
			0x0957e5c4, 0x12afcb88, 0x255f9710, 0x4abf2e20, 0x957e5c40, 0x2afcb881, 0x55f97102, 0xabf2e204,
			0x57e5c409, 0xafcb8812, 0x5f971025, 0xbf2e204a, 0x7e5c4095, 0xfcb8812a, 0xf9710255, 0xf2e204ab,
			0xe5c40957,	0xcb8812af, 0x9710255f, 0x2e204abf}
		};




// Initialization for LEA
// Set LEA parameters(master key info.) and generate round keys in lea_t.
//
// input : LEA_st, key, bit length of key
// output: LEA_st
// return: 0 or -1(error, undefined key length)
int LEA_set_init_params(LEA_st *lea, const uint8_t *key, const uint32_t key_bit_length)
{
	uint32_t K[8] = { 0, };
	uint32_t key_byte_length = (key_bit_length >> 3);

	LEA_reset(lea);


	memcpy(lea->K, key, key_byte_length);


	lea->K_byte_length = key_byte_length;

	K[0] = *(uint32_t*)(lea->K     );
	K[1] = *(uint32_t*)(lea->K +  4);
	K[2] = *(uint32_t*)(lea->K +  8);
	K[3] = *(uint32_t*)(lea->K + 12);

	switch (key_bit_length)
	{
	case 128:
		lea->Nr = 24;

		lea->RK[  0] = ROL32(K[0]         + delta[0][ 0], 1);
		lea->RK[  6] = ROL32(lea->RK[  0] + delta[1][ 1], 1);
		lea->RK[ 12] = ROL32(lea->RK[  6] + delta[2][ 2], 1);
		lea->RK[ 18] = ROL32(lea->RK[ 12] + delta[3][ 3], 1);
		lea->RK[ 24] = ROL32(lea->RK[ 18] + delta[0][ 4], 1);
		lea->RK[ 30] = ROL32(lea->RK[ 24] + delta[1][ 5], 1);
		lea->RK[ 36] = ROL32(lea->RK[ 30] + delta[2][ 6], 1);
		lea->RK[ 42] = ROL32(lea->RK[ 36] + delta[3][ 7], 1);
		lea->RK[ 48] = ROL32(lea->RK[ 42] + delta[0][ 8], 1);
		lea->RK[ 54] = ROL32(lea->RK[ 48] + delta[1][ 9], 1);
		lea->RK[ 60] = ROL32(lea->RK[ 54] + delta[2][10], 1);
		lea->RK[ 66] = ROL32(lea->RK[ 60] + delta[3][11], 1);
		lea->RK[ 72] = ROL32(lea->RK[ 66] + delta[0][12], 1);
		lea->RK[ 78] = ROL32(lea->RK[ 72] + delta[1][13], 1);
		lea->RK[ 84] = ROL32(lea->RK[ 78] + delta[2][14], 1);
		lea->RK[ 90] = ROL32(lea->RK[ 84] + delta[3][15], 1);
		lea->RK[ 96] = ROL32(lea->RK[ 90] + delta[0][16], 1);
		lea->RK[102] = ROL32(lea->RK[ 96] + delta[1][17], 1);
		lea->RK[108] = ROL32(lea->RK[102] + delta[2][18], 1);
		lea->RK[114] = ROL32(lea->RK[108] + delta[3][19], 1);
		lea->RK[120] = ROL32(lea->RK[114] + delta[0][20], 1);
		lea->RK[126] = ROL32(lea->RK[120] + delta[1][21], 1);
		lea->RK[132] = ROL32(lea->RK[126] + delta[2][22], 1);
		lea->RK[138] = ROL32(lea->RK[132] + delta[3][23], 1);

		lea->RK[  1] = lea->RK[  3] = lea->RK[  5] = ROL32(K[1]         + delta[0][ 1], 3);
		lea->RK[  7] = lea->RK[  9] = lea->RK[ 11] = ROL32(lea->RK[  1] + delta[1][ 2], 3);
		lea->RK[ 13] = lea->RK[ 15] = lea->RK[ 17] = ROL32(lea->RK[  7] + delta[2][ 3], 3);
		lea->RK[ 19] = lea->RK[ 21] = lea->RK[ 23] = ROL32(lea->RK[ 13] + delta[3][ 4], 3);
		lea->RK[ 25] = lea->RK[ 27] = lea->RK[ 29] = ROL32(lea->RK[ 19] + delta[0][ 5], 3);
		lea->RK[ 31] = lea->RK[ 33] = lea->RK[ 35] = ROL32(lea->RK[ 25] + delta[1][ 6], 3);
		lea->RK[ 37] = lea->RK[ 39] = lea->RK[ 41] = ROL32(lea->RK[ 31] + delta[2][ 7], 3);
		lea->RK[ 43] = lea->RK[ 45] = lea->RK[ 47] = ROL32(lea->RK[ 37] + delta[3][ 8], 3);
		lea->RK[ 49] = lea->RK[ 51] = lea->RK[ 53] = ROL32(lea->RK[ 43] + delta[0][ 9], 3);
		lea->RK[ 55] = lea->RK[ 57] = lea->RK[ 59] = ROL32(lea->RK[ 49] + delta[1][10], 3);
		lea->RK[ 61] = lea->RK[ 63] = lea->RK[ 65] = ROL32(lea->RK[ 55] + delta[2][11], 3);
		lea->RK[ 67] = lea->RK[ 69] = lea->RK[ 71] = ROL32(lea->RK[ 61] + delta[3][12], 3);
		lea->RK[ 73] = lea->RK[ 75] = lea->RK[ 77] = ROL32(lea->RK[ 67] + delta[0][13], 3);
		lea->RK[ 79] = lea->RK[ 81] = lea->RK[ 83] = ROL32(lea->RK[ 73] + delta[1][14], 3);
		lea->RK[ 85] = lea->RK[ 87] = lea->RK[ 89] = ROL32(lea->RK[ 79] + delta[2][15], 3);
		lea->RK[ 91] = lea->RK[ 93] = lea->RK[ 95] = ROL32(lea->RK[ 85] + delta[3][16], 3);
		lea->RK[ 97] = lea->RK[ 99] = lea->RK[101] = ROL32(lea->RK[ 91] + delta[0][17], 3);
		lea->RK[103] = lea->RK[105] = lea->RK[107] = ROL32(lea->RK[ 97] + delta[1][18], 3);
		lea->RK[109] = lea->RK[111] = lea->RK[113] = ROL32(lea->RK[103] + delta[2][19], 3);
		lea->RK[115] = lea->RK[117] = lea->RK[119] = ROL32(lea->RK[109] + delta[3][20], 3);
		lea->RK[121] = lea->RK[123] = lea->RK[125] = ROL32(lea->RK[115] + delta[0][21], 3);
		lea->RK[127] = lea->RK[129] = lea->RK[131] = ROL32(lea->RK[121] + delta[1][22], 3);
		lea->RK[133] = lea->RK[135] = lea->RK[137] = ROL32(lea->RK[127] + delta[2][23], 3);
		lea->RK[139] = lea->RK[141] = lea->RK[143] = ROL32(lea->RK[133] + delta[3][24], 3);

		lea->RK[  2] = ROL32(K[2]         + delta[0][ 2], 6);
		lea->RK[  8] = ROL32(lea->RK[  2] + delta[1][ 3], 6);
		lea->RK[ 14] = ROL32(lea->RK[  8] + delta[2][ 4], 6);
		lea->RK[ 20] = ROL32(lea->RK[ 14] + delta[3][ 5], 6);
		lea->RK[ 26] = ROL32(lea->RK[ 20] + delta[0][ 6], 6);
		lea->RK[ 32] = ROL32(lea->RK[ 26] + delta[1][ 7], 6);
		lea->RK[ 38] = ROL32(lea->RK[ 32] + delta[2][ 8], 6);
		lea->RK[ 44] = ROL32(lea->RK[ 38] + delta[3][ 9], 6);
		lea->RK[ 50] = ROL32(lea->RK[ 44] + delta[0][10], 6);
		lea->RK[ 56] = ROL32(lea->RK[ 50] + delta[1][11], 6);
		lea->RK[ 62] = ROL32(lea->RK[ 56] + delta[2][12], 6);
		lea->RK[ 68] = ROL32(lea->RK[ 62] + delta[3][13], 6);
		lea->RK[ 74] = ROL32(lea->RK[ 68] + delta[0][14], 6);
		lea->RK[ 80] = ROL32(lea->RK[ 74] + delta[1][15], 6);
		lea->RK[ 86] = ROL32(lea->RK[ 80] + delta[2][16], 6);
		lea->RK[ 92] = ROL32(lea->RK[ 86] + delta[3][17], 6);
		lea->RK[ 98] = ROL32(lea->RK[ 92] + delta[0][18], 6);
		lea->RK[104] = ROL32(lea->RK[ 98] + delta[1][19], 6);
		lea->RK[110] = ROL32(lea->RK[104] + delta[2][20], 6);
		lea->RK[116] = ROL32(lea->RK[110] + delta[3][21], 6);
		lea->RK[122] = ROL32(lea->RK[116] + delta[0][22], 6);
		lea->RK[128] = ROL32(lea->RK[122] + delta[1][23], 6);
		lea->RK[134] = ROL32(lea->RK[128] + delta[2][24], 6);
		lea->RK[140] = ROL32(lea->RK[134] + delta[3][25], 6);

		lea->RK[  4] = ROL32(K[3]         + delta[0][ 3], 11);
		lea->RK[ 10] = ROL32(lea->RK[  4] + delta[1][ 4], 11);
		lea->RK[ 16] = ROL32(lea->RK[ 10] + delta[2][ 5], 11);
		lea->RK[ 22] = ROL32(lea->RK[ 16] + delta[3][ 6], 11);
		lea->RK[ 28] = ROL32(lea->RK[ 22] + delta[0][ 7], 11);
		lea->RK[ 34] = ROL32(lea->RK[ 28] + delta[1][ 8], 11);
		lea->RK[ 40] = ROL32(lea->RK[ 34] + delta[2][ 9], 11);
		lea->RK[ 46] = ROL32(lea->RK[ 40] + delta[3][10], 11);
		lea->RK[ 52] = ROL32(lea->RK[ 46] + delta[0][11], 11);
		lea->RK[ 58] = ROL32(lea->RK[ 52] + delta[1][12], 11);
		lea->RK[ 64] = ROL32(lea->RK[ 58] + delta[2][13], 11);
		lea->RK[ 70] = ROL32(lea->RK[ 64] + delta[3][14], 11);
		lea->RK[ 76] = ROL32(lea->RK[ 70] + delta[0][15], 11);
		lea->RK[ 82] = ROL32(lea->RK[ 76] + delta[1][16], 11);
		lea->RK[ 88] = ROL32(lea->RK[ 82] + delta[2][17], 11);
		lea->RK[ 94] = ROL32(lea->RK[ 88] + delta[3][18], 11);
		lea->RK[100] = ROL32(lea->RK[ 94] + delta[0][19], 11);
		lea->RK[106] = ROL32(lea->RK[100] + delta[1][20], 11);
		lea->RK[112] = ROL32(lea->RK[106] + delta[2][21], 11);
		lea->RK[118] = ROL32(lea->RK[112] + delta[3][22], 11);
		lea->RK[124] = ROL32(lea->RK[118] + delta[0][23], 11);
		lea->RK[130] = ROL32(lea->RK[124] + delta[1][24], 11);
		lea->RK[136] = ROL32(lea->RK[130] + delta[2][25], 11);
		lea->RK[142] = ROL32(lea->RK[136] + delta[3][26], 11);
		break;

	case 192:
		lea->Nr = 28;
		K[4] = *(uint32_t*)(lea->K + 16);
		K[5] = *(uint32_t*)(lea->K + 20);

		lea->RK[  0] = ROL32(K[0]         + delta[0][ 0], 1);
		lea->RK[  6] = ROL32(lea->RK[  0] + delta[1][ 1], 1);
		lea->RK[ 12] = ROL32(lea->RK[  6] + delta[2][ 2], 1);
		lea->RK[ 18] = ROL32(lea->RK[ 12] + delta[3][ 3], 1);
		lea->RK[ 24] = ROL32(lea->RK[ 18] + delta[4][ 4], 1);
		lea->RK[ 30] = ROL32(lea->RK[ 24] + delta[5][ 5], 1);
		lea->RK[ 36] = ROL32(lea->RK[ 30] + delta[0][ 6], 1);
		lea->RK[ 42] = ROL32(lea->RK[ 36] + delta[1][ 7], 1);
		lea->RK[ 48] = ROL32(lea->RK[ 42] + delta[2][ 8], 1);
		lea->RK[ 54] = ROL32(lea->RK[ 48] + delta[3][ 9], 1);
		lea->RK[ 60] = ROL32(lea->RK[ 54] + delta[4][10], 1);
		lea->RK[ 66] = ROL32(lea->RK[ 60] + delta[5][11], 1);
		lea->RK[ 72] = ROL32(lea->RK[ 66] + delta[0][12], 1);
		lea->RK[ 78] = ROL32(lea->RK[ 72] + delta[1][13], 1);
		lea->RK[ 84] = ROL32(lea->RK[ 78] + delta[2][14], 1);
		lea->RK[ 90] = ROL32(lea->RK[ 84] + delta[3][15], 1);
		lea->RK[ 96] = ROL32(lea->RK[ 90] + delta[4][16], 1);
		lea->RK[102] = ROL32(lea->RK[ 96] + delta[5][17], 1);
		lea->RK[108] = ROL32(lea->RK[102] + delta[0][18], 1);
		lea->RK[114] = ROL32(lea->RK[108] + delta[1][19], 1);
		lea->RK[120] = ROL32(lea->RK[114] + delta[2][20], 1);
		lea->RK[126] = ROL32(lea->RK[120] + delta[3][21], 1);
		lea->RK[132] = ROL32(lea->RK[126] + delta[4][22], 1);
		lea->RK[138] = ROL32(lea->RK[132] + delta[5][23], 1);
		lea->RK[144] = ROL32(lea->RK[138] + delta[0][24], 1);
		lea->RK[150] = ROL32(lea->RK[144] + delta[1][25], 1);
		lea->RK[156] = ROL32(lea->RK[150] + delta[2][26], 1);
		lea->RK[162] = ROL32(lea->RK[156] + delta[3][27], 1);

		lea->RK[  1] = ROL32(K[1]         + delta[0][ 1], 3);
		lea->RK[  7] = ROL32(lea->RK[  1] + delta[1][ 2], 3);
		lea->RK[ 13] = ROL32(lea->RK[  7] + delta[2][ 3], 3);
		lea->RK[ 19] = ROL32(lea->RK[ 13] + delta[3][ 4], 3);
		lea->RK[ 25] = ROL32(lea->RK[ 19] + delta[4][ 5], 3);
		lea->RK[ 31] = ROL32(lea->RK[ 25] + delta[5][ 6], 3);
		lea->RK[ 37] = ROL32(lea->RK[ 31] + delta[0][ 7], 3);
		lea->RK[ 43] = ROL32(lea->RK[ 37] + delta[1][ 8], 3);
		lea->RK[ 49] = ROL32(lea->RK[ 43] + delta[2][ 9], 3);
		lea->RK[ 55] = ROL32(lea->RK[ 49] + delta[3][10], 3);
		lea->RK[ 61] = ROL32(lea->RK[ 55] + delta[4][11], 3);
		lea->RK[ 67] = ROL32(lea->RK[ 61] + delta[5][12], 3);
		lea->RK[ 73] = ROL32(lea->RK[ 67] + delta[0][13], 3);
		lea->RK[ 79] = ROL32(lea->RK[ 73] + delta[1][14], 3);
		lea->RK[ 85] = ROL32(lea->RK[ 79] + delta[2][15], 3);
		lea->RK[ 91] = ROL32(lea->RK[ 85] + delta[3][16], 3);
		lea->RK[ 97] = ROL32(lea->RK[ 91] + delta[4][17], 3);
		lea->RK[103] = ROL32(lea->RK[ 97] + delta[5][18], 3);
		lea->RK[109] = ROL32(lea->RK[103] + delta[0][19], 3);
		lea->RK[115] = ROL32(lea->RK[109] + delta[1][20], 3);
		lea->RK[121] = ROL32(lea->RK[115] + delta[2][21], 3);
		lea->RK[127] = ROL32(lea->RK[121] + delta[3][22], 3);
		lea->RK[133] = ROL32(lea->RK[127] + delta[4][23], 3);
		lea->RK[139] = ROL32(lea->RK[133] + delta[5][24], 3);
		lea->RK[145] = ROL32(lea->RK[139] + delta[0][25], 3);
		lea->RK[151] = ROL32(lea->RK[145] + delta[1][26], 3);
		lea->RK[157] = ROL32(lea->RK[151] + delta[2][27], 3);
		lea->RK[163] = ROL32(lea->RK[157] + delta[3][28], 3);

		lea->RK[  2] = ROL32(K[2]         + delta[0][ 2], 6);
		lea->RK[  8] = ROL32(lea->RK[  2] + delta[1][ 3], 6);
		lea->RK[ 14] = ROL32(lea->RK[  8] + delta[2][ 4], 6);
		lea->RK[ 20] = ROL32(lea->RK[ 14] + delta[3][ 5], 6);
		lea->RK[ 26] = ROL32(lea->RK[ 20] + delta[4][ 6], 6);
		lea->RK[ 32] = ROL32(lea->RK[ 26] + delta[5][ 7], 6);
		lea->RK[ 38] = ROL32(lea->RK[ 32] + delta[0][ 8], 6);
		lea->RK[ 44] = ROL32(lea->RK[ 38] + delta[1][ 9], 6);
		lea->RK[ 50] = ROL32(lea->RK[ 44] + delta[2][10], 6);
		lea->RK[ 56] = ROL32(lea->RK[ 50] + delta[3][11], 6);
		lea->RK[ 62] = ROL32(lea->RK[ 56] + delta[4][12], 6);
		lea->RK[ 68] = ROL32(lea->RK[ 62] + delta[5][13], 6);
		lea->RK[ 74] = ROL32(lea->RK[ 68] + delta[0][14], 6);
		lea->RK[ 80] = ROL32(lea->RK[ 74] + delta[1][15], 6);
		lea->RK[ 86] = ROL32(lea->RK[ 80] + delta[2][16], 6);
		lea->RK[ 92] = ROL32(lea->RK[ 86] + delta[3][17], 6);
		lea->RK[ 98] = ROL32(lea->RK[ 92] + delta[4][18], 6);
		lea->RK[104] = ROL32(lea->RK[ 98] + delta[5][19], 6);
		lea->RK[110] = ROL32(lea->RK[104] + delta[0][20], 6);
		lea->RK[116] = ROL32(lea->RK[110] + delta[1][21], 6);
		lea->RK[122] = ROL32(lea->RK[116] + delta[2][22], 6);
		lea->RK[128] = ROL32(lea->RK[122] + delta[3][23], 6);
		lea->RK[134] = ROL32(lea->RK[128] + delta[4][24], 6);
		lea->RK[140] = ROL32(lea->RK[134] + delta[5][25], 6);
		lea->RK[146] = ROL32(lea->RK[140] + delta[0][26], 6);
		lea->RK[152] = ROL32(lea->RK[146] + delta[1][27], 6);
		lea->RK[158] = ROL32(lea->RK[152] + delta[2][28], 6);
		lea->RK[164] = ROL32(lea->RK[158] + delta[3][29], 6);

		lea->RK[  3] = ROL32(K[3]         + delta[0][ 3], 11);
		lea->RK[  9] = ROL32(lea->RK[  3] + delta[1][ 4], 11);
		lea->RK[ 15] = ROL32(lea->RK[  9] + delta[2][ 5], 11);
		lea->RK[ 21] = ROL32(lea->RK[ 15] + delta[3][ 6], 11);
		lea->RK[ 27] = ROL32(lea->RK[ 21] + delta[4][ 7], 11);
		lea->RK[ 33] = ROL32(lea->RK[ 27] + delta[5][ 8], 11);
		lea->RK[ 39] = ROL32(lea->RK[ 33] + delta[0][ 9], 11);
		lea->RK[ 45] = ROL32(lea->RK[ 39] + delta[1][10], 11);
		lea->RK[ 51] = ROL32(lea->RK[ 45] + delta[2][11], 11);
		lea->RK[ 57] = ROL32(lea->RK[ 51] + delta[3][12], 11);
		lea->RK[ 63] = ROL32(lea->RK[ 57] + delta[4][13], 11);
		lea->RK[ 69] = ROL32(lea->RK[ 63] + delta[5][14], 11);
		lea->RK[ 75] = ROL32(lea->RK[ 69] + delta[0][15], 11);
		lea->RK[ 81] = ROL32(lea->RK[ 75] + delta[1][16], 11);
		lea->RK[ 87] = ROL32(lea->RK[ 81] + delta[2][17], 11);
		lea->RK[ 93] = ROL32(lea->RK[ 87] + delta[3][18], 11);
		lea->RK[ 99] = ROL32(lea->RK[ 93] + delta[4][19], 11);
		lea->RK[105] = ROL32(lea->RK[ 99] + delta[5][20], 11);
		lea->RK[111] = ROL32(lea->RK[105] + delta[0][21], 11);
		lea->RK[117] = ROL32(lea->RK[111] + delta[1][22], 11);
		lea->RK[123] = ROL32(lea->RK[117] + delta[2][23], 11);
		lea->RK[129] = ROL32(lea->RK[123] + delta[3][24], 11);
		lea->RK[135] = ROL32(lea->RK[129] + delta[4][25], 11);
		lea->RK[141] = ROL32(lea->RK[135] + delta[5][26], 11);
		lea->RK[147] = ROL32(lea->RK[141] + delta[0][27], 11);
		lea->RK[153] = ROL32(lea->RK[147] + delta[1][28], 11);
		lea->RK[159] = ROL32(lea->RK[153] + delta[2][29], 11);
		lea->RK[165] = ROL32(lea->RK[159] + delta[3][30], 11);

		lea->RK[  4] = ROL32(K[4]         + delta[0][ 4], 13);
		lea->RK[ 10] = ROL32(lea->RK[  4] + delta[1][ 5], 13);
		lea->RK[ 16] = ROL32(lea->RK[ 10] + delta[2][ 6], 13);
		lea->RK[ 22] = ROL32(lea->RK[ 16] + delta[3][ 7], 13);
		lea->RK[ 28] = ROL32(lea->RK[ 22] + delta[4][ 8], 13);
		lea->RK[ 34] = ROL32(lea->RK[ 28] + delta[5][ 9], 13);
		lea->RK[ 40] = ROL32(lea->RK[ 34] + delta[0][10], 13);
		lea->RK[ 46] = ROL32(lea->RK[ 40] + delta[1][11], 13);
		lea->RK[ 52] = ROL32(lea->RK[ 46] + delta[2][12], 13);
		lea->RK[ 58] = ROL32(lea->RK[ 52] + delta[3][13], 13);
		lea->RK[ 64] = ROL32(lea->RK[ 58] + delta[4][14], 13);
		lea->RK[ 70] = ROL32(lea->RK[ 64] + delta[5][15], 13);
		lea->RK[ 76] = ROL32(lea->RK[ 70] + delta[0][16], 13);
		lea->RK[ 82] = ROL32(lea->RK[ 76] + delta[1][17], 13);
		lea->RK[ 88] = ROL32(lea->RK[ 82] + delta[2][18], 13);
		lea->RK[ 94] = ROL32(lea->RK[ 88] + delta[3][19], 13);
		lea->RK[100] = ROL32(lea->RK[ 94] + delta[4][20], 13);
		lea->RK[106] = ROL32(lea->RK[100] + delta[5][21], 13);
		lea->RK[112] = ROL32(lea->RK[106] + delta[0][22], 13);
		lea->RK[118] = ROL32(lea->RK[112] + delta[1][23], 13);
		lea->RK[124] = ROL32(lea->RK[118] + delta[2][24], 13);
		lea->RK[130] = ROL32(lea->RK[124] + delta[3][25], 13);
		lea->RK[136] = ROL32(lea->RK[130] + delta[4][26], 13);
		lea->RK[142] = ROL32(lea->RK[136] + delta[5][27], 13);
		lea->RK[148] = ROL32(lea->RK[142] + delta[0][28], 13);
		lea->RK[154] = ROL32(lea->RK[148] + delta[1][29], 13);
		lea->RK[160] = ROL32(lea->RK[154] + delta[2][30], 13);
		lea->RK[166] = ROL32(lea->RK[160] + delta[3][31], 13);

		lea->RK[  5] = ROL32(K[5]         + delta[0][ 5], 17);
		lea->RK[ 11] = ROL32(lea->RK[  5] + delta[1][ 6], 17);
		lea->RK[ 17] = ROL32(lea->RK[ 11] + delta[2][ 7], 17);
		lea->RK[ 23] = ROL32(lea->RK[ 17] + delta[3][ 8], 17);
		lea->RK[ 29] = ROL32(lea->RK[ 23] + delta[4][ 9], 17);
		lea->RK[ 35] = ROL32(lea->RK[ 29] + delta[5][10], 17);
		lea->RK[ 41] = ROL32(lea->RK[ 35] + delta[0][11], 17);
		lea->RK[ 47] = ROL32(lea->RK[ 41] + delta[1][12], 17);
		lea->RK[ 53] = ROL32(lea->RK[ 47] + delta[2][13], 17);
		lea->RK[ 59] = ROL32(lea->RK[ 53] + delta[3][14], 17);
		lea->RK[ 65] = ROL32(lea->RK[ 59] + delta[4][15], 17);
		lea->RK[ 71] = ROL32(lea->RK[ 65] + delta[5][16], 17);
		lea->RK[ 77] = ROL32(lea->RK[ 71] + delta[0][17], 17);
		lea->RK[ 83] = ROL32(lea->RK[ 77] + delta[1][18], 17);
		lea->RK[ 89] = ROL32(lea->RK[ 83] + delta[2][19], 17);
		lea->RK[ 95] = ROL32(lea->RK[ 89] + delta[3][20], 17);
		lea->RK[101] = ROL32(lea->RK[ 95] + delta[4][21], 17);
		lea->RK[107] = ROL32(lea->RK[101] + delta[5][22], 17);
		lea->RK[113] = ROL32(lea->RK[107] + delta[0][23], 17);
		lea->RK[119] = ROL32(lea->RK[113] + delta[1][24], 17);
		lea->RK[125] = ROL32(lea->RK[119] + delta[2][25], 17);
		lea->RK[131] = ROL32(lea->RK[125] + delta[3][26], 17);
		lea->RK[137] = ROL32(lea->RK[131] + delta[4][27], 17);
		lea->RK[143] = ROL32(lea->RK[137] + delta[5][28], 17);
		lea->RK[149] = ROL32(lea->RK[143] + delta[0][29], 17);
		lea->RK[155] = ROL32(lea->RK[149] + delta[1][30], 17);
		lea->RK[161] = ROL32(lea->RK[155] + delta[2][31], 17);
		lea->RK[167] = ROL32(lea->RK[161] + delta[3][ 0], 17);
		break;

	case 256:
		lea->Nr = 32;

		K[4] = *(uint32_t*)(lea->K + 16);
		K[5] = *(uint32_t*)(lea->K + 20);
		K[6] = *(uint32_t*)(lea->K + 24);
		K[7] = *(uint32_t*)(lea->K + 28);

		lea->RK[  0] = ROL32(K[0]         + delta[0][ 0],  1);
		lea->RK[  8] = ROL32(lea->RK[  0] + delta[1][ 3],  6);
		lea->RK[ 16] = ROL32(lea->RK[  8] + delta[2][ 6], 13);
		lea->RK[ 24] = ROL32(lea->RK[ 16] + delta[4][ 4],  1);
		lea->RK[ 32] = ROL32(lea->RK[ 24] + delta[5][ 7],  6);
		lea->RK[ 40] = ROL32(lea->RK[ 32] + delta[6][10], 13);
		lea->RK[ 48] = ROL32(lea->RK[ 40] + delta[0][ 8],  1);
		lea->RK[ 56] = ROL32(lea->RK[ 48] + delta[1][11],  6);
		lea->RK[ 64] = ROL32(lea->RK[ 56] + delta[2][14], 13);
		lea->RK[ 72] = ROL32(lea->RK[ 64] + delta[4][12],  1);
		lea->RK[ 80] = ROL32(lea->RK[ 72] + delta[5][15],  6);
		lea->RK[ 88] = ROL32(lea->RK[ 80] + delta[6][18], 13);
		lea->RK[ 96] = ROL32(lea->RK[ 88] + delta[0][16],  1);
		lea->RK[104] = ROL32(lea->RK[ 96] + delta[1][19],  6);
		lea->RK[112] = ROL32(lea->RK[104] + delta[2][22], 13);
		lea->RK[120] = ROL32(lea->RK[112] + delta[4][20],  1);
		lea->RK[128] = ROL32(lea->RK[120] + delta[5][23],  6);
		lea->RK[136] = ROL32(lea->RK[128] + delta[6][26], 13);
		lea->RK[144] = ROL32(lea->RK[136] + delta[0][24],  1);
		lea->RK[152] = ROL32(lea->RK[144] + delta[1][27],  6);
		lea->RK[160] = ROL32(lea->RK[152] + delta[2][30], 13);
		lea->RK[168] = ROL32(lea->RK[160] + delta[4][28],  1);
		lea->RK[176] = ROL32(lea->RK[168] + delta[5][31],  6);
		lea->RK[184] = ROL32(lea->RK[176] + delta[6][ 2], 13);

		lea->RK[  1] = ROL32(K[1]         + delta[0][ 1],  3);
		lea->RK[  9] = ROL32(lea->RK[  1] + delta[1][ 4], 11);
		lea->RK[ 17] = ROL32(lea->RK[  9] + delta[2][ 7], 17);
		lea->RK[ 25] = ROL32(lea->RK[ 17] + delta[4][ 5],  3);
		lea->RK[ 33] = ROL32(lea->RK[ 25] + delta[5][ 8], 11);
		lea->RK[ 41] = ROL32(lea->RK[ 33] + delta[6][11], 17);
		lea->RK[ 49] = ROL32(lea->RK[ 41] + delta[0][ 9],  3);
		lea->RK[ 57] = ROL32(lea->RK[ 49] + delta[1][12], 11);
		lea->RK[ 65] = ROL32(lea->RK[ 57] + delta[2][15], 17);
		lea->RK[ 73] = ROL32(lea->RK[ 65] + delta[4][13],  3);
		lea->RK[ 81] = ROL32(lea->RK[ 73] + delta[5][16], 11);
		lea->RK[ 89] = ROL32(lea->RK[ 81] + delta[6][19], 17);
		lea->RK[ 97] = ROL32(lea->RK[ 89] + delta[0][17],  3);
		lea->RK[105] = ROL32(lea->RK[ 97] + delta[1][20], 11);
		lea->RK[113] = ROL32(lea->RK[105] + delta[2][23], 17);
		lea->RK[121] = ROL32(lea->RK[113] + delta[4][21],  3);
		lea->RK[129] = ROL32(lea->RK[121] + delta[5][24], 11);
		lea->RK[137] = ROL32(lea->RK[129] + delta[6][27], 17);
		lea->RK[145] = ROL32(lea->RK[137] + delta[0][25],  3);
		lea->RK[153] = ROL32(lea->RK[145] + delta[1][28], 11);
		lea->RK[161] = ROL32(lea->RK[153] + delta[2][31], 17);
		lea->RK[169] = ROL32(lea->RK[161] + delta[4][29],  3);
		lea->RK[177] = ROL32(lea->RK[169] + delta[5][ 0], 11);
		lea->RK[185] = ROL32(lea->RK[177] + delta[6][ 3], 17);

		lea->RK[  2] = ROL32(K[2]         + delta[0][ 2],  6);
		lea->RK[ 10] = ROL32(lea->RK[  2] + delta[1][ 5], 13);
		lea->RK[ 18] = ROL32(lea->RK[ 10] + delta[3][ 3],  1);
		lea->RK[ 26] = ROL32(lea->RK[ 18] + delta[4][ 6],  6);
		lea->RK[ 34] = ROL32(lea->RK[ 26] + delta[5][ 9], 13);
		lea->RK[ 42] = ROL32(lea->RK[ 34] + delta[7][ 7],  1);
		lea->RK[ 50] = ROL32(lea->RK[ 42] + delta[0][10],  6);
		lea->RK[ 58] = ROL32(lea->RK[ 50] + delta[1][13], 13);
		lea->RK[ 66] = ROL32(lea->RK[ 58] + delta[3][11],  1);
		lea->RK[ 74] = ROL32(lea->RK[ 66] + delta[4][14],  6);
		lea->RK[ 82] = ROL32(lea->RK[ 74] + delta[5][17], 13);
		lea->RK[ 90] = ROL32(lea->RK[ 82] + delta[7][15],  1);
		lea->RK[ 98] = ROL32(lea->RK[ 90] + delta[0][18],  6);
		lea->RK[106] = ROL32(lea->RK[ 98] + delta[1][21], 13);
		lea->RK[114] = ROL32(lea->RK[106] + delta[3][19],  1);
		lea->RK[122] = ROL32(lea->RK[114] + delta[4][22],  6);
		lea->RK[130] = ROL32(lea->RK[122] + delta[5][25], 13);
		lea->RK[138] = ROL32(lea->RK[130] + delta[7][23],  1);
		lea->RK[146] = ROL32(lea->RK[138] + delta[0][26],  6);
		lea->RK[154] = ROL32(lea->RK[146] + delta[1][29], 13);
		lea->RK[162] = ROL32(lea->RK[154] + delta[3][27],  1);
		lea->RK[170] = ROL32(lea->RK[162] + delta[4][30],  6);
		lea->RK[178] = ROL32(lea->RK[170] + delta[5][ 1], 13);
		lea->RK[186] = ROL32(lea->RK[178] + delta[7][31],  1);

		lea->RK[  3] = ROL32(K[3]         + delta[0][ 3], 11);
		lea->RK[ 11] = ROL32(lea->RK[  3] + delta[1][ 6], 17);
		lea->RK[ 19] = ROL32(lea->RK[ 11] + delta[3][ 4],  3);
		lea->RK[ 27] = ROL32(lea->RK[ 19] + delta[4][ 7], 11);
		lea->RK[ 35] = ROL32(lea->RK[ 27] + delta[5][10], 17);
		lea->RK[ 43] = ROL32(lea->RK[ 35] + delta[7][ 8],  3);
		lea->RK[ 51] = ROL32(lea->RK[ 43] + delta[0][11], 11);
		lea->RK[ 59] = ROL32(lea->RK[ 51] + delta[1][14], 17);
		lea->RK[ 67] = ROL32(lea->RK[ 59] + delta[3][12],  3);
		lea->RK[ 75] = ROL32(lea->RK[ 67] + delta[4][15], 11);
		lea->RK[ 83] = ROL32(lea->RK[ 75] + delta[5][18], 17);
		lea->RK[ 91] = ROL32(lea->RK[ 83] + delta[7][16],  3);
		lea->RK[ 99] = ROL32(lea->RK[ 91] + delta[0][19], 11);
		lea->RK[107] = ROL32(lea->RK[ 99] + delta[1][22], 17);
		lea->RK[115] = ROL32(lea->RK[107] + delta[3][20],  3);
		lea->RK[123] = ROL32(lea->RK[115] + delta[4][23], 11);
		lea->RK[131] = ROL32(lea->RK[123] + delta[5][26], 17);
		lea->RK[139] = ROL32(lea->RK[131] + delta[7][24],  3);
		lea->RK[147] = ROL32(lea->RK[139] + delta[0][27], 11);
		lea->RK[155] = ROL32(lea->RK[147] + delta[1][30], 17);
		lea->RK[163] = ROL32(lea->RK[155] + delta[3][28],  3);
		lea->RK[171] = ROL32(lea->RK[163] + delta[4][31], 11);
		lea->RK[179] = ROL32(lea->RK[171] + delta[5][ 2], 17);
		lea->RK[187] = ROL32(lea->RK[179] + delta[7][ 0],  3);

		lea->RK[  4] = ROL32(K[4]         + delta[0][ 4], 13);
		lea->RK[ 12] = ROL32(lea->RK[  4] + delta[2][ 2],  1);
		lea->RK[ 20] = ROL32(lea->RK[ 12] + delta[3][ 5],  6);
		lea->RK[ 28] = ROL32(lea->RK[ 20] + delta[4][ 8], 13);
		lea->RK[ 36] = ROL32(lea->RK[ 28] + delta[6][ 6],  1);
		lea->RK[ 44] = ROL32(lea->RK[ 36] + delta[7][ 9],  6);
		lea->RK[ 52] = ROL32(lea->RK[ 44] + delta[0][12], 13);
		lea->RK[ 60] = ROL32(lea->RK[ 52] + delta[2][10],  1);
		lea->RK[ 68] = ROL32(lea->RK[ 60] + delta[3][13],  6);
		lea->RK[ 76] = ROL32(lea->RK[ 68] + delta[4][16], 13);
		lea->RK[ 84] = ROL32(lea->RK[ 76] + delta[6][14],  1);
		lea->RK[ 92] = ROL32(lea->RK[ 84] + delta[7][17],  6);
		lea->RK[100] = ROL32(lea->RK[ 92] + delta[0][20], 13);
		lea->RK[108] = ROL32(lea->RK[100] + delta[2][18],  1);
		lea->RK[116] = ROL32(lea->RK[108] + delta[3][21],  6);
		lea->RK[124] = ROL32(lea->RK[116] + delta[4][24], 13);
		lea->RK[132] = ROL32(lea->RK[124] + delta[6][22],  1);
		lea->RK[140] = ROL32(lea->RK[132] + delta[7][25],  6);
		lea->RK[148] = ROL32(lea->RK[140] + delta[0][28], 13);
		lea->RK[156] = ROL32(lea->RK[148] + delta[2][26],  1);
		lea->RK[164] = ROL32(lea->RK[156] + delta[3][29],  6);
		lea->RK[172] = ROL32(lea->RK[164] + delta[4][ 0], 13);
		lea->RK[180] = ROL32(lea->RK[172] + delta[6][30],  1);
		lea->RK[188] = ROL32(lea->RK[180] + delta[7][ 1],  6);

		lea->RK[  5] = ROL32(K[5]         + delta[0][ 5], 17);
		lea->RK[ 13] = ROL32(lea->RK[  5] + delta[2][ 3],  3);
		lea->RK[ 21] = ROL32(lea->RK[ 13] + delta[3][ 6], 11);
		lea->RK[ 29] = ROL32(lea->RK[ 21] + delta[4][ 9], 17);
		lea->RK[ 37] = ROL32(lea->RK[ 29] + delta[6][ 7],  3);
		lea->RK[ 45] = ROL32(lea->RK[ 37] + delta[7][10], 11);
		lea->RK[ 53] = ROL32(lea->RK[ 45] + delta[0][13], 17);
		lea->RK[ 61] = ROL32(lea->RK[ 53] + delta[2][11],  3);
		lea->RK[ 69] = ROL32(lea->RK[ 61] + delta[3][14], 11);
		lea->RK[ 77] = ROL32(lea->RK[ 69] + delta[4][17], 17);
		lea->RK[ 85] = ROL32(lea->RK[ 77] + delta[6][15],  3);
		lea->RK[ 93] = ROL32(lea->RK[ 85] + delta[7][18], 11);
		lea->RK[101] = ROL32(lea->RK[ 93] + delta[0][21], 17);
		lea->RK[109] = ROL32(lea->RK[101] + delta[2][19],  3);
		lea->RK[117] = ROL32(lea->RK[109] + delta[3][22], 11);
		lea->RK[125] = ROL32(lea->RK[117] + delta[4][25], 17);
		lea->RK[133] = ROL32(lea->RK[125] + delta[6][23],  3);
		lea->RK[141] = ROL32(lea->RK[133] + delta[7][26], 11);
		lea->RK[149] = ROL32(lea->RK[141] + delta[0][29], 17);
		lea->RK[157] = ROL32(lea->RK[149] + delta[2][27],  3);
		lea->RK[165] = ROL32(lea->RK[157] + delta[3][30], 11);
		lea->RK[173] = ROL32(lea->RK[165] + delta[4][ 1], 17);
		lea->RK[181] = ROL32(lea->RK[173] + delta[6][31],  3);
		lea->RK[189] = ROL32(lea->RK[181] + delta[7][ 2], 11);

		lea->RK[  6] = ROL32(K[6]         + delta[1][ 1],  1);
		lea->RK[ 14] = ROL32(lea->RK[  6] + delta[2][ 4],  6);
		lea->RK[ 22] = ROL32(lea->RK[ 14] + delta[3][ 7], 13);
		lea->RK[ 30] = ROL32(lea->RK[ 22] + delta[5][ 5],  1);
		lea->RK[ 38] = ROL32(lea->RK[ 30] + delta[6][ 8],  6);
		lea->RK[ 46] = ROL32(lea->RK[ 38] + delta[7][11], 13);
		lea->RK[ 54] = ROL32(lea->RK[ 46] + delta[1][ 9],  1);
		lea->RK[ 62] = ROL32(lea->RK[ 54] + delta[2][12],  6);
		lea->RK[ 70] = ROL32(lea->RK[ 62] + delta[3][15], 13);
		lea->RK[ 78] = ROL32(lea->RK[ 70] + delta[5][13],  1);
		lea->RK[ 86] = ROL32(lea->RK[ 78] + delta[6][16],  6);
		lea->RK[ 94] = ROL32(lea->RK[ 86] + delta[7][19], 13);
		lea->RK[102] = ROL32(lea->RK[ 94] + delta[1][17],  1);
		lea->RK[110] = ROL32(lea->RK[102] + delta[2][20],  6);
		lea->RK[118] = ROL32(lea->RK[110] + delta[3][23], 13);
		lea->RK[126] = ROL32(lea->RK[118] + delta[5][21],  1);
		lea->RK[134] = ROL32(lea->RK[126] + delta[6][24],  6);
		lea->RK[142] = ROL32(lea->RK[134] + delta[7][27], 13);
		lea->RK[150] = ROL32(lea->RK[142] + delta[1][25],  1);
		lea->RK[158] = ROL32(lea->RK[150] + delta[2][28],  6);
		lea->RK[166] = ROL32(lea->RK[158] + delta[3][31], 13);
		lea->RK[174] = ROL32(lea->RK[166] + delta[5][29],  1);
		lea->RK[182] = ROL32(lea->RK[174] + delta[6][ 0],  6);
		lea->RK[190] = ROL32(lea->RK[182] + delta[7][ 3], 13);

		lea->RK[  7] = ROL32(K[7]         + delta[1][ 2],  3);
		lea->RK[ 15] = ROL32(lea->RK[  7] + delta[2][ 5], 11);
		lea->RK[ 23] = ROL32(lea->RK[ 15] + delta[3][ 8], 17);
		lea->RK[ 31] = ROL32(lea->RK[ 23] + delta[5][ 6],  3);
		lea->RK[ 39] = ROL32(lea->RK[ 31] + delta[6][ 9], 11);
		lea->RK[ 47] = ROL32(lea->RK[ 39] + delta[7][12], 17);
		lea->RK[ 55] = ROL32(lea->RK[ 47] + delta[1][10],  3);
		lea->RK[ 63] = ROL32(lea->RK[ 55] + delta[2][13], 11);
		lea->RK[ 71] = ROL32(lea->RK[ 63] + delta[3][16], 17);
		lea->RK[ 79] = ROL32(lea->RK[ 71] + delta[5][14],  3);
		lea->RK[ 87] = ROL32(lea->RK[ 79] + delta[6][17], 11);
		lea->RK[ 95] = ROL32(lea->RK[ 87] + delta[7][20], 17);
		lea->RK[103] = ROL32(lea->RK[ 95] + delta[1][18],  3);
		lea->RK[111] = ROL32(lea->RK[103] + delta[2][21], 11);
		lea->RK[119] = ROL32(lea->RK[111] + delta[3][24], 17);
		lea->RK[127] = ROL32(lea->RK[119] + delta[5][22],  3);
		lea->RK[135] = ROL32(lea->RK[127] + delta[6][25], 11);
		lea->RK[143] = ROL32(lea->RK[135] + delta[7][28], 17);
		lea->RK[151] = ROL32(lea->RK[143] + delta[1][26],  3);
		lea->RK[159] = ROL32(lea->RK[151] + delta[2][29], 11);
		lea->RK[167] = ROL32(lea->RK[159] + delta[3][ 0], 17);
		lea->RK[175] = ROL32(lea->RK[167] + delta[5][30],  3);
		lea->RK[183] = ROL32(lea->RK[175] + delta[6][ 1], 11);
		lea->RK[191] = ROL32(lea->RK[183] + delta[7][ 4], 17);
		break;

	default:
		return -1; // undefined key length
	}

	return 0;
}


// Reset the LEA data structure
//
// input : LEA_st
// output: LEA_st
// return: None
void LEA_reset(LEA_st *lea)
{
	memset(lea->K, 0, 32);
	lea->K_byte_length = 0;

	memset(lea->RK, 0, 768);
	lea->Nr = 0;

	memset(lea->P, 0, 16);

	memset(lea->C, 0, 16);
}



// LEA encryption
// Based on NSR & KISA LEA reference code for optimization!!!
//
// input : LEA_st
// output: LEA_st
// return: None
void LEA_enc(LEA_st *lea)
{
	register uint32_t X0, X1, X2, X3;
	register uint8_t *input  = lea->P;
	register uint8_t *output = lea->C;

	X0 = *(uint32_t*)(input     );
	X1 = *(uint32_t*)(input +  4);
	X2 = *(uint32_t*)(input +  8);
	X3 = *(uint32_t*)(input + 12);

	X3 = ROR32((X2 ^ lea->RK[  4]) + (X3 ^ lea->RK[  5]), 3);
	X2 = ROR32((X1 ^ lea->RK[  2]) + (X2 ^ lea->RK[  3]), 5);
	X1 = ROL32((X0 ^ lea->RK[  0]) + (X1 ^ lea->RK[  1]), 9);
	X0 = ROR32((X3 ^ lea->RK[ 10]) + (X0 ^ lea->RK[ 11]), 3);
	X3 = ROR32((X2 ^ lea->RK[  8]) + (X3 ^ lea->RK[  9]), 5);
	X2 = ROL32((X1 ^ lea->RK[  6]) + (X2 ^ lea->RK[  7]), 9);
	X1 = ROR32((X0 ^ lea->RK[ 16]) + (X1 ^ lea->RK[ 17]), 3);
	X0 = ROR32((X3 ^ lea->RK[ 14]) + (X0 ^ lea->RK[ 15]), 5);
	X3 = ROL32((X2 ^ lea->RK[ 12]) + (X3 ^ lea->RK[ 13]), 9);
	X2 = ROR32((X1 ^ lea->RK[ 22]) + (X2 ^ lea->RK[ 23]), 3);
	X1 = ROR32((X0 ^ lea->RK[ 20]) + (X1 ^ lea->RK[ 21]), 5);
	X0 = ROL32((X3 ^ lea->RK[ 18]) + (X0 ^ lea->RK[ 19]), 9);

	X3 = ROR32((X2 ^ lea->RK[ 28]) + (X3 ^ lea->RK[ 29]), 3);
	X2 = ROR32((X1 ^ lea->RK[ 26]) + (X2 ^ lea->RK[ 27]), 5);
	X1 = ROL32((X0 ^ lea->RK[ 24]) + (X1 ^ lea->RK[ 25]), 9);
	X0 = ROR32((X3 ^ lea->RK[ 34]) + (X0 ^ lea->RK[ 35]), 3);
	X3 = ROR32((X2 ^ lea->RK[ 32]) + (X3 ^ lea->RK[ 33]), 5);
	X2 = ROL32((X1 ^ lea->RK[ 30]) + (X2 ^ lea->RK[ 31]), 9);
	X1 = ROR32((X0 ^ lea->RK[ 40]) + (X1 ^ lea->RK[ 41]), 3);
	X0 = ROR32((X3 ^ lea->RK[ 38]) + (X0 ^ lea->RK[ 39]), 5);
	X3 = ROL32((X2 ^ lea->RK[ 36]) + (X3 ^ lea->RK[ 37]), 9);
	X2 = ROR32((X1 ^ lea->RK[ 46]) + (X2 ^ lea->RK[ 47]), 3);
	X1 = ROR32((X0 ^ lea->RK[ 44]) + (X1 ^ lea->RK[ 45]), 5);
	X0 = ROL32((X3 ^ lea->RK[ 42]) + (X0 ^ lea->RK[ 43]), 9);

	X3 = ROR32((X2 ^ lea->RK[ 52]) + (X3 ^ lea->RK[ 53]), 3);
	X2 = ROR32((X1 ^ lea->RK[ 50]) + (X2 ^ lea->RK[ 51]), 5);
	X1 = ROL32((X0 ^ lea->RK[ 48]) + (X1 ^ lea->RK[ 49]), 9);
	X0 = ROR32((X3 ^ lea->RK[ 58]) + (X0 ^ lea->RK[ 59]), 3);
	X3 = ROR32((X2 ^ lea->RK[ 56]) + (X3 ^ lea->RK[ 57]), 5);
	X2 = ROL32((X1 ^ lea->RK[ 54]) + (X2 ^ lea->RK[ 55]), 9);
	X1 = ROR32((X0 ^ lea->RK[ 64]) + (X1 ^ lea->RK[ 65]), 3);
	X0 = ROR32((X3 ^ lea->RK[ 62]) + (X0 ^ lea->RK[ 63]), 5);
	X3 = ROL32((X2 ^ lea->RK[ 60]) + (X3 ^ lea->RK[ 61]), 9);
	X2 = ROR32((X1 ^ lea->RK[ 70]) + (X2 ^ lea->RK[ 71]), 3);
	X1 = ROR32((X0 ^ lea->RK[ 68]) + (X1 ^ lea->RK[ 69]), 5);
	X0 = ROL32((X3 ^ lea->RK[ 66]) + (X0 ^ lea->RK[ 67]), 9);

	X3 = ROR32((X2 ^ lea->RK[ 76]) + (X3 ^ lea->RK[ 77]), 3);
	X2 = ROR32((X1 ^ lea->RK[ 74]) + (X2 ^ lea->RK[ 75]), 5);
	X1 = ROL32((X0 ^ lea->RK[ 72]) + (X1 ^ lea->RK[ 73]), 9);
	X0 = ROR32((X3 ^ lea->RK[ 82]) + (X0 ^ lea->RK[ 83]), 3);
	X3 = ROR32((X2 ^ lea->RK[ 80]) + (X3 ^ lea->RK[ 81]), 5);
	X2 = ROL32((X1 ^ lea->RK[ 78]) + (X2 ^ lea->RK[ 79]), 9);
	X1 = ROR32((X0 ^ lea->RK[ 88]) + (X1 ^ lea->RK[ 89]), 3);
	X0 = ROR32((X3 ^ lea->RK[ 86]) + (X0 ^ lea->RK[ 87]), 5);
	X3 = ROL32((X2 ^ lea->RK[ 84]) + (X3 ^ lea->RK[ 85]), 9);
	X2 = ROR32((X1 ^ lea->RK[ 94]) + (X2 ^ lea->RK[ 95]), 3);
	X1 = ROR32((X0 ^ lea->RK[ 92]) + (X1 ^ lea->RK[ 93]), 5);
	X0 = ROL32((X3 ^ lea->RK[ 90]) + (X0 ^ lea->RK[ 91]), 9);

	X3 = ROR32((X2 ^ lea->RK[100]) + (X3 ^ lea->RK[101]), 3);
	X2 = ROR32((X1 ^ lea->RK[ 98]) + (X2 ^ lea->RK[ 99]), 5);
	X1 = ROL32((X0 ^ lea->RK[ 96]) + (X1 ^ lea->RK[ 97]), 9);
	X0 = ROR32((X3 ^ lea->RK[106]) + (X0 ^ lea->RK[107]), 3);
	X3 = ROR32((X2 ^ lea->RK[104]) + (X3 ^ lea->RK[105]), 5);
	X2 = ROL32((X1 ^ lea->RK[102]) + (X2 ^ lea->RK[103]), 9);
	X1 = ROR32((X0 ^ lea->RK[112]) + (X1 ^ lea->RK[113]), 3);
	X0 = ROR32((X3 ^ lea->RK[110]) + (X0 ^ lea->RK[111]), 5);
	X3 = ROL32((X2 ^ lea->RK[108]) + (X3 ^ lea->RK[109]), 9);
	X2 = ROR32((X1 ^ lea->RK[118]) + (X2 ^ lea->RK[119]), 3);
	X1 = ROR32((X0 ^ lea->RK[116]) + (X1 ^ lea->RK[117]), 5);
	X0 = ROL32((X3 ^ lea->RK[114]) + (X0 ^ lea->RK[115]), 9);

	X3 = ROR32((X2 ^ lea->RK[124]) + (X3 ^ lea->RK[125]), 3);
	X2 = ROR32((X1 ^ lea->RK[122]) + (X2 ^ lea->RK[123]), 5);
	X1 = ROL32((X0 ^ lea->RK[120]) + (X1 ^ lea->RK[121]), 9);
	X0 = ROR32((X3 ^ lea->RK[130]) + (X0 ^ lea->RK[131]), 3);
	X3 = ROR32((X2 ^ lea->RK[128]) + (X3 ^ lea->RK[129]), 5);
	X2 = ROL32((X1 ^ lea->RK[126]) + (X2 ^ lea->RK[127]), 9);
	X1 = ROR32((X0 ^ lea->RK[136]) + (X1 ^ lea->RK[137]), 3);
	X0 = ROR32((X3 ^ lea->RK[134]) + (X0 ^ lea->RK[135]), 5);
	X3 = ROL32((X2 ^ lea->RK[132]) + (X3 ^ lea->RK[133]), 9);
	X2 = ROR32((X1 ^ lea->RK[142]) + (X2 ^ lea->RK[143]), 3);
	X1 = ROR32((X0 ^ lea->RK[140]) + (X1 ^ lea->RK[141]), 5);
	X0 = ROL32((X3 ^ lea->RK[138]) + (X0 ^ lea->RK[139]), 9);

	if(lea->Nr >= 28)
	{
		X3 = ROR32((X2 ^ lea->RK[148]) + (X3 ^ lea->RK[149]), 3);
		X2 = ROR32((X1 ^ lea->RK[146]) + (X2 ^ lea->RK[147]), 5);
		X1 = ROL32((X0 ^ lea->RK[144]) + (X1 ^ lea->RK[145]), 9);
		X0 = ROR32((X3 ^ lea->RK[154]) + (X0 ^ lea->RK[155]), 3);
		X3 = ROR32((X2 ^ lea->RK[152]) + (X3 ^ lea->RK[153]), 5);
		X2 = ROL32((X1 ^ lea->RK[150]) + (X2 ^ lea->RK[151]), 9);
		X1 = ROR32((X0 ^ lea->RK[160]) + (X1 ^ lea->RK[161]), 3);
		X0 = ROR32((X3 ^ lea->RK[158]) + (X0 ^ lea->RK[159]), 5);
		X3 = ROL32((X2 ^ lea->RK[156]) + (X3 ^ lea->RK[157]), 9);
		X2 = ROR32((X1 ^ lea->RK[166]) + (X2 ^ lea->RK[167]), 3);
		X1 = ROR32((X0 ^ lea->RK[164]) + (X1 ^ lea->RK[165]), 5);
		X0 = ROL32((X3 ^ lea->RK[162]) + (X0 ^ lea->RK[163]), 9);

		if(lea->Nr == 32)
		{
			X3 = ROR32((X2 ^ lea->RK[172]) + (X3 ^ lea->RK[173]), 3);
			X2 = ROR32((X1 ^ lea->RK[170]) + (X2 ^ lea->RK[171]), 5);
			X1 = ROL32((X0 ^ lea->RK[168]) + (X1 ^ lea->RK[169]), 9);
			X0 = ROR32((X3 ^ lea->RK[178]) + (X0 ^ lea->RK[179]), 3);
			X3 = ROR32((X2 ^ lea->RK[176]) + (X3 ^ lea->RK[177]), 5);
			X2 = ROL32((X1 ^ lea->RK[174]) + (X2 ^ lea->RK[175]), 9);
			X1 = ROR32((X0 ^ lea->RK[184]) + (X1 ^ lea->RK[185]), 3);
			X0 = ROR32((X3 ^ lea->RK[182]) + (X0 ^ lea->RK[183]), 5);
			X3 = ROL32((X2 ^ lea->RK[180]) + (X3 ^ lea->RK[181]), 9);
			X2 = ROR32((X1 ^ lea->RK[190]) + (X2 ^ lea->RK[191]), 3);
			X1 = ROR32((X0 ^ lea->RK[188]) + (X1 ^ lea->RK[189]), 5);
			X0 = ROL32((X3 ^ lea->RK[186]) + (X0 ^ lea->RK[187]), 9);
		}
	}

	*(uint32_t*)(output     ) = X0;
	*(uint32_t*)(output +  4) = X1;
	*(uint32_t*)(output +  8) = X2;
	*(uint32_t*)(output + 12) = X3;
}



// LEA decrypt
// Based on NSR & KISA LEA reference code for optimization!!!
//
// input : LEA_st
// output: LEA_st
// return: None
void LEA_dec(LEA_st *lea)
{
	register uint32_t X0, X1, X2, X3;
	register uint8_t *input  = lea->C;
	register uint8_t *output = lea->P;

	X0 = *(uint32_t*)(input     );
	X1 = *(uint32_t*)(input +  4);
	X2 = *(uint32_t*)(input +  8);
	X3 = *(uint32_t*)(input + 12);

	if(lea->Nr >= 28)
	{
		if(lea->Nr == 32)
		{
			X0 = (ROR32(X0, 9) - (X3 ^ lea->RK[186])) ^ lea->RK[187];
			X1 = (ROL32(X1, 5) - (X0 ^ lea->RK[188])) ^ lea->RK[189];
			X2 = (ROL32(X2, 3) - (X1 ^ lea->RK[190])) ^ lea->RK[191];
			X3 = (ROR32(X3, 9) - (X2 ^ lea->RK[180])) ^ lea->RK[181];
			X0 = (ROL32(X0, 5) - (X3 ^ lea->RK[182])) ^ lea->RK[183];
			X1 = (ROL32(X1, 3) - (X0 ^ lea->RK[184])) ^ lea->RK[185];
			X2 = (ROR32(X2, 9) - (X1 ^ lea->RK[174])) ^ lea->RK[175];
			X3 = (ROL32(X3, 5) - (X2 ^ lea->RK[176])) ^ lea->RK[177];
			X0 = (ROL32(X0, 3) - (X3 ^ lea->RK[178])) ^ lea->RK[179];
			X1 = (ROR32(X1, 9) - (X0 ^ lea->RK[168])) ^ lea->RK[169];
			X2 = (ROL32(X2, 5) - (X1 ^ lea->RK[170])) ^ lea->RK[171];
			X3 = (ROL32(X3, 3) - (X2 ^ lea->RK[172])) ^ lea->RK[173];
		}

		X0 = (ROR32(X0, 9) - (X3 ^ lea->RK[162])) ^ lea->RK[163];
		X1 = (ROL32(X1, 5) - (X0 ^ lea->RK[164])) ^ lea->RK[165];
		X2 = (ROL32(X2, 3) - (X1 ^ lea->RK[166])) ^ lea->RK[167];
		X3 = (ROR32(X3, 9) - (X2 ^ lea->RK[156])) ^ lea->RK[157];
		X0 = (ROL32(X0, 5) - (X3 ^ lea->RK[158])) ^ lea->RK[159];
		X1 = (ROL32(X1, 3) - (X0 ^ lea->RK[160])) ^ lea->RK[161];
		X2 = (ROR32(X2, 9) - (X1 ^ lea->RK[150])) ^ lea->RK[151];
		X3 = (ROL32(X3, 5) - (X2 ^ lea->RK[152])) ^ lea->RK[153];
		X0 = (ROL32(X0, 3) - (X3 ^ lea->RK[154])) ^ lea->RK[155];
		X1 = (ROR32(X1, 9) - (X0 ^ lea->RK[144])) ^ lea->RK[145];
		X2 = (ROL32(X2, 5) - (X1 ^ lea->RK[146])) ^ lea->RK[147];
		X3 = (ROL32(X3, 3) - (X2 ^ lea->RK[148])) ^ lea->RK[149];
	}

	X0 = (ROR32(X0, 9) - (X3 ^ lea->RK[138])) ^ lea->RK[139];
	X1 = (ROL32(X1, 5) - (X0 ^ lea->RK[140])) ^ lea->RK[141];
	X2 = (ROL32(X2, 3) - (X1 ^ lea->RK[142])) ^ lea->RK[143];
	X3 = (ROR32(X3, 9) - (X2 ^ lea->RK[132])) ^ lea->RK[133];
	X0 = (ROL32(X0, 5) - (X3 ^ lea->RK[134])) ^ lea->RK[135];
	X1 = (ROL32(X1, 3) - (X0 ^ lea->RK[136])) ^ lea->RK[137];
	X2 = (ROR32(X2, 9) - (X1 ^ lea->RK[126])) ^ lea->RK[127];
	X3 = (ROL32(X3, 5) - (X2 ^ lea->RK[128])) ^ lea->RK[129];
	X0 = (ROL32(X0, 3) - (X3 ^ lea->RK[130])) ^ lea->RK[131];
	X1 = (ROR32(X1, 9) - (X0 ^ lea->RK[120])) ^ lea->RK[121];
	X2 = (ROL32(X2, 5) - (X1 ^ lea->RK[122])) ^ lea->RK[123];
	X3 = (ROL32(X3, 3) - (X2 ^ lea->RK[124])) ^ lea->RK[125];

	X0 = (ROR32(X0, 9) - (X3 ^ lea->RK[114])) ^ lea->RK[115];
	X1 = (ROL32(X1, 5) - (X0 ^ lea->RK[116])) ^ lea->RK[117];
	X2 = (ROL32(X2, 3) - (X1 ^ lea->RK[118])) ^ lea->RK[119];
	X3 = (ROR32(X3, 9) - (X2 ^ lea->RK[108])) ^ lea->RK[109];
	X0 = (ROL32(X0, 5) - (X3 ^ lea->RK[110])) ^ lea->RK[111];
	X1 = (ROL32(X1, 3) - (X0 ^ lea->RK[112])) ^ lea->RK[113];
	X2 = (ROR32(X2, 9) - (X1 ^ lea->RK[102])) ^ lea->RK[103];
	X3 = (ROL32(X3, 5) - (X2 ^ lea->RK[104])) ^ lea->RK[105];
	X0 = (ROL32(X0, 3) - (X3 ^ lea->RK[106])) ^ lea->RK[107];
	X1 = (ROR32(X1, 9) - (X0 ^ lea->RK[ 96])) ^ lea->RK[ 97];
	X2 = (ROL32(X2, 5) - (X1 ^ lea->RK[ 98])) ^ lea->RK[ 99];
	X3 = (ROL32(X3, 3) - (X2 ^ lea->RK[100])) ^ lea->RK[101];

	X0 = (ROR32(X0, 9) - (X3 ^ lea->RK[ 90])) ^ lea->RK[ 91];
	X1 = (ROL32(X1, 5) - (X0 ^ lea->RK[ 92])) ^ lea->RK[ 93];
	X2 = (ROL32(X2, 3) - (X1 ^ lea->RK[ 94])) ^ lea->RK[ 95];
	X3 = (ROR32(X3, 9) - (X2 ^ lea->RK[ 84])) ^ lea->RK[ 85];
	X0 = (ROL32(X0, 5) - (X3 ^ lea->RK[ 86])) ^ lea->RK[ 87];
	X1 = (ROL32(X1, 3) - (X0 ^ lea->RK[ 88])) ^ lea->RK[ 89];
	X2 = (ROR32(X2, 9) - (X1 ^ lea->RK[ 78])) ^ lea->RK[ 79];
	X3 = (ROL32(X3, 5) - (X2 ^ lea->RK[ 80])) ^ lea->RK[ 81];
	X0 = (ROL32(X0, 3) - (X3 ^ lea->RK[ 82])) ^ lea->RK[ 83];
	X1 = (ROR32(X1, 9) - (X0 ^ lea->RK[ 72])) ^ lea->RK[ 73];
	X2 = (ROL32(X2, 5) - (X1 ^ lea->RK[ 74])) ^ lea->RK[ 75];
	X3 = (ROL32(X3, 3) - (X2 ^ lea->RK[ 76])) ^ lea->RK[ 77];

	X0 = (ROR32(X0, 9) - (X3 ^ lea->RK[ 66])) ^ lea->RK[ 67];
	X1 = (ROL32(X1, 5) - (X0 ^ lea->RK[ 68])) ^ lea->RK[ 69];
	X2 = (ROL32(X2, 3) - (X1 ^ lea->RK[ 70])) ^ lea->RK[ 71];
	X3 = (ROR32(X3, 9) - (X2 ^ lea->RK[ 60])) ^ lea->RK[ 61];
	X0 = (ROL32(X0, 5) - (X3 ^ lea->RK[ 62])) ^ lea->RK[ 63];
	X1 = (ROL32(X1, 3) - (X0 ^ lea->RK[ 64])) ^ lea->RK[ 65];
	X2 = (ROR32(X2, 9) - (X1 ^ lea->RK[ 54])) ^ lea->RK[ 55];
	X3 = (ROL32(X3, 5) - (X2 ^ lea->RK[ 56])) ^ lea->RK[ 57];
	X0 = (ROL32(X0, 3) - (X3 ^ lea->RK[ 58])) ^ lea->RK[ 59];
	X1 = (ROR32(X1, 9) - (X0 ^ lea->RK[ 48])) ^ lea->RK[ 49];
	X2 = (ROL32(X2, 5) - (X1 ^ lea->RK[ 50])) ^ lea->RK[ 51];
	X3 = (ROL32(X3, 3) - (X2 ^ lea->RK[ 52])) ^ lea->RK[ 53];

	X0 = (ROR32(X0, 9) - (X3 ^ lea->RK[ 42])) ^ lea->RK[ 43];
	X1 = (ROL32(X1, 5) - (X0 ^ lea->RK[ 44])) ^ lea->RK[ 45];
	X2 = (ROL32(X2, 3) - (X1 ^ lea->RK[ 46])) ^ lea->RK[ 47];
	X3 = (ROR32(X3, 9) - (X2 ^ lea->RK[ 36])) ^ lea->RK[ 37];
	X0 = (ROL32(X0, 5) - (X3 ^ lea->RK[ 38])) ^ lea->RK[ 39];
	X1 = (ROL32(X1, 3) - (X0 ^ lea->RK[ 40])) ^ lea->RK[ 41];
	X2 = (ROR32(X2, 9) - (X1 ^ lea->RK[ 30])) ^ lea->RK[ 31];
	X3 = (ROL32(X3, 5) - (X2 ^ lea->RK[ 32])) ^ lea->RK[ 33];
	X0 = (ROL32(X0, 3) - (X3 ^ lea->RK[ 34])) ^ lea->RK[ 35];
	X1 = (ROR32(X1, 9) - (X0 ^ lea->RK[ 24])) ^ lea->RK[ 25];
	X2 = (ROL32(X2, 5) - (X1 ^ lea->RK[ 26])) ^ lea->RK[ 27];
	X3 = (ROL32(X3, 3) - (X2 ^ lea->RK[ 28])) ^ lea->RK[ 29];

	X0 = (ROR32(X0, 9) - (X3 ^ lea->RK[ 18])) ^ lea->RK[ 19];
	X1 = (ROL32(X1, 5) - (X0 ^ lea->RK[ 20])) ^ lea->RK[ 21];
	X2 = (ROL32(X2, 3) - (X1 ^ lea->RK[ 22])) ^ lea->RK[ 23];
	X3 = (ROR32(X3, 9) - (X2 ^ lea->RK[ 12])) ^ lea->RK[ 13];
	X0 = (ROL32(X0, 5) - (X3 ^ lea->RK[ 14])) ^ lea->RK[ 15];
	X1 = (ROL32(X1, 3) - (X0 ^ lea->RK[ 16])) ^ lea->RK[ 17];
	X2 = (ROR32(X2, 9) - (X1 ^ lea->RK[  6])) ^ lea->RK[  7];
	X3 = (ROL32(X3, 5) - (X2 ^ lea->RK[  8])) ^ lea->RK[  9];
	X0 = (ROL32(X0, 3) - (X3 ^ lea->RK[ 10])) ^ lea->RK[ 11];
	X1 = (ROR32(X1, 9) - (X0 ^ lea->RK[  0])) ^ lea->RK[  1];
	X2 = (ROL32(X2, 5) - (X1 ^ lea->RK[  2])) ^ lea->RK[  3];
	X3 = (ROL32(X3, 3) - (X2 ^ lea->RK[  4])) ^ lea->RK[  5];

	*(uint32_t*)(output     ) = X0;
	*(uint32_t*)(output +  4) = X1;
	*(uint32_t*)(output +  8) = X2;
	*(uint32_t*)(output + 12) = X3;
}

// End of lea.c
