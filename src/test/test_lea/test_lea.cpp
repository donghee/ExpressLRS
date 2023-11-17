#include <cstdint>
#include <iostream>
#include <fstream>
#include <unity.h>
#include "common.h"

using namespace std;

extern "C"
{
#include "ccm4lea.h"
#include "gcm4lea.h"
}

void test_gcm_enc_dec(void)
{
    GCM_st gcm_TX;
    GCM_st gcm_RX;

    CCM_st ccm_TX;
    CCM_st ccm_RX;

    uint8_t K[16] = {0x14, 0x87, 0x0B, 0x99, 0x92, 0xEA, 0x89, 0x67, 0x8A, 0x1D, 0xDF, 0xD6, 0x30, 0x91, 0x8D, 0xF0};
    uint8_t A[16] = { 0, };
    uint8_t N[12] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B};
    uint8_t control_data[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};

    uint8_t payload[32];
    TEST_ASSERT_EQUAL(0, GCM4LEA_set_init_params(&gcm_TX, K, 128, A, 0, 128));
	TEST_ASSERT_EQUAL(0, GCM4LEA_set_init_params(&gcm_RX, K, 128, A, 0, 128));

	// TX
    TEST_ASSERT_EQUAL(0, GCM4LEA_set_enc_params(&gcm_TX, control_data, 16, N, 12));
    TEST_ASSERT_EQUAL(0, GCM4LEA_enc(&gcm_TX));


    uint8_t p_T[] = {0x9A, 0x5F, 0xA8, 0x80, 0x2F, 0x42, 0x1D, 0xAE, 0xFC, 0x22, 0x57, 0x6F, 0xE5, 0x48, 0x3D, 0xC7};
    TEST_ASSERT_EQUAL_HEX8_ARRAY(p_T, gcm_TX.T, 16);

    uint8_t p_CC[] = {0xB3, 0x84, 0x4F, 0x48, 0x8B, 0x21, 0x3d, 0x53, 0x9E, 0x6C, 0x4B, 0xB8, 0x3E, 0x6A, 0x18, 0xC5};
    TEST_ASSERT_EQUAL_HEX8_ARRAY(p_CC, gcm_TX.CC, 16);


	// RX
	memcpy(payload, gcm_TX.T, 16);
	memcpy(payload+16, gcm_TX.CC, 16);

	TEST_ASSERT_EQUAL(0, GCM4LEA_set_dec_params(&gcm_RX, payload+16, 16, N, 12, payload));
    TEST_ASSERT_EQUAL(0, GCM4LEA_dec(&gcm_RX));

    TEST_ASSERT_EQUAL_HEX8_ARRAY(control_data, gcm_RX.PP, gcm_RX.PP_byte_length);

    // std::ofstream f("/tmp/gcm4lea.log");
    // for (int i; i < 16; i++)
    // {
    //     f << gcm_TX.T[i] << " ";
    // }
}

#define CONTROL_DATA_SIZE 64

void test_gcm_enc_dec_64(void)
{
    GCM_st gcm_TX;
    GCM_st gcm_RX;

    CCM_st ccm_TX;
    CCM_st ccm_RX;

    // key?
    uint8_t K[16] = {0x14, 0x87, 0x0B, 0x99, 0x92, 0xEA, 0x89, 0x67, 0x8A, 0x1D, 0xDF, 0xD6, 0x30, 0x91, 0x8D, 0xF0};
    uint8_t A[16] = { 0, };
    uint8_t N[12] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B};
    uint8_t control_data[CONTROL_DATA_SIZE] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
                                0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
                                0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
                                0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F
    };

    uint8_t payload[CONTROL_DATA_SIZE*2];
    TEST_ASSERT_EQUAL(0, GCM4LEA_set_init_params(&gcm_TX, K, 128, A, 0, 128));
	TEST_ASSERT_EQUAL(0, GCM4LEA_set_init_params(&gcm_RX, K, 128, A, 0, 128));

	// TX
    TEST_ASSERT_EQUAL(0, GCM4LEA_set_enc_params(&gcm_TX, control_data, CONTROL_DATA_SIZE, N, 12));
    TEST_ASSERT_EQUAL(0, GCM4LEA_enc(&gcm_TX));

	// RX
	memcpy(payload, gcm_TX.T, CONTROL_DATA_SIZE);
	memcpy(payload+CONTROL_DATA_SIZE, gcm_TX.CC, CONTROL_DATA_SIZE);

	TEST_ASSERT_EQUAL(0, GCM4LEA_set_dec_params(&gcm_RX, payload+CONTROL_DATA_SIZE, CONTROL_DATA_SIZE, N, 12, payload));
    TEST_ASSERT_EQUAL(0, GCM4LEA_dec(&gcm_RX));

    TEST_ASSERT_EQUAL_HEX8_ARRAY(control_data, gcm_RX.PP, gcm_RX.PP_byte_length);

}

void test_ccm_enc_dec()
{
    CCM_st ccm_TX;
    CCM_st ccm_RX;

    uint8_t K[16] = {0x14, 0x87, 0x0B, 0x99, 0x92, 0xEA, 0x89, 0x67, 0x8A, 0x1D, 0xDF, 0xD6, 0x30, 0x91, 0x8D, 0xF0};
    uint8_t A[16] = { 0, };
    uint8_t N[12] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B};

    uint8_t control_data[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
    uint8_t p_control_data[16] = {};

    uint8_t payload[32];


    // Init CCM parameters
	TEST_ASSERT_EQUAL(0, CCM4LEA_set_init_params(&ccm_TX, K, 128, A, 0, 16));
	TEST_ASSERT_EQUAL(0, CCM4LEA_set_init_params(&ccm_RX, K, 128, A, 0, 16));

	// TX
	TEST_ASSERT_EQUAL(0, CCM4LEA_set_enc_params(&ccm_TX, control_data, 16, N, 12));
	TEST_ASSERT_EQUAL(0, CCM4LEA_enc(&ccm_TX));

	memcpy(payload, ccm_TX.T, 16);
	memcpy(payload+16, ccm_TX.CC, 16);

    // TODO: WHY payload is changed?
	memcpy(p_control_data, control_data, 16);

	// RX
    TEST_ASSERT_EQUAL(0, CCM4LEA_set_dec_params(&ccm_RX, payload+16, 16, N, 12, payload));
    TEST_ASSERT_EQUAL(0, CCM4LEA_dec(&ccm_RX));

    TEST_ASSERT_EQUAL_HEX8_ARRAY(p_control_data, ccm_RX.PP, ccm_RX.PP_byte_length);
}

void setUp() {}
void tearDown() {}

int main(int argc, char **argv)
{
    UNITY_BEGIN();
    RUN_TEST(test_gcm_enc_dec);
    RUN_TEST(test_gcm_enc_dec_64);
    RUN_TEST(test_ccm_enc_dec);
    UNITY_END();

    return 0;
}
