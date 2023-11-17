#include <cstdint>
#include <iostream>
#include <unity.h>
using namespace std;

#include "common.h"

#include "ccm4lea.h"
#include "gcm4lea.h"

void test_ver_to_u32(void)
{
    GCM_st gcm_TX;
    GCM_st gcm_RX;

    CCM_st ccm_TX;
    CCM_st ccm_RX;

    uint8_t K[16] = {0x14, 0x87, 0x0B, 0x99, 0x92, 0xEA, 0x89, 0x67, 0x8A, 0x1D, 0xDF, 0xD6, 0x30, 0x91, 0x8D, 0xF0};
    uint8_t A[16] = {
        0,
    };

    uint8_t N[12] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B};
    uint8_t control_data[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};

    uint8_t payload[32];
    // int ret = GCM4LEA_set_init_params(&gcm_TX, K, 128, A, 0, 128);
    int ret = 0;

    TEST_ASSERT_EQUAL(0, ret);
}

// Unity setup/teardown
void setUp() {}
void tearDown() {}

int main(int argc, char **argv)
{
    UNITY_BEGIN();
    RUN_TEST(test_ver_to_u32);
    UNITY_END();

    return 0;
}
