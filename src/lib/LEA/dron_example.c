// Example of usage instructions for drone control data encryption
//
// by Joungil Yun (sigipus@etri.re.kr)
//
// 2023.10.

#include "ccm4lea.h"
#include "gcm4lea.h"


int dron_example()
{
    int i;
	GCM_st gcm_TX;
	GCM_st gcm_RX;

	CCM_st ccm_TX;
	CCM_st ccm_RX;

    uint8_t K[16] = {0x14, 0x87, 0x0B, 0x99, 0x92, 0xEA, 0x89, 0x67, 0x8A, 0x1D, 0xDF, 0xD6, 0x30, 0x91, 0x8D, 0xF0};
    uint8_t A[16] = {0, };

    uint8_t N[12] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B };
    uint8_t control_data[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};

    uint8_t payload[32];

    //
    // GCM Example
    //

    // Invoke initialization once on the TX!
    if(GCM4LEA_set_init_params(&gcm_TX, K, 128, A, 0, 128) < 0) return -1; // Note that it is the length of T in bits, not bytes

    // Invoke initialization once on the RX!
	if(GCM4LEA_set_init_params(&gcm_RX, K, 128, A, 0, 128) < 0) return -1; // Note that it is the length of T in bits, not bytes


	// TX
    if(GCM4LEA_set_enc_params(&gcm_TX, control_data, 16, N, 12) < 0) return -1;
    if(GCM4LEA_enc(&gcm_TX) < 0) return -1;

	memcpy(payload, gcm_TX.T, 16);
	memcpy(payload+16, gcm_TX.CC, 16);


	// RX
	if(GCM4LEA_set_dec_params(&gcm_RX, payload+16, 16, N, 12, payload) < 0) return -1;
	if(GCM4LEA_dec(&gcm_RX) < 0) return -1; // Mismatch of T


	// The following is for result verification purposes only.
	CONSOL_PRINTF("GCM [Received  T]: ");
	for(i = 0; i < 16; i++) CONSOL_PRINTF("0x%02x ", payload[i]);
	CONSOL_PRINTF("\n");

	CONSOL_PRINTF("GCM [Generated T]: ");
	for(i = 0; i < 16; i++) CONSOL_PRINTF("0x%02x ", gcm_RX.T[i]);
	CONSOL_PRINTF("\n\n");

	CONSOL_PRINTF("GCM [Decrypted control data]: ");
	for (i = 0; i < gcm_RX.PP_byte_length; i++) CONSOL_PRINTF("0x%02x ", gcm_RX.PP[i]);
	CONSOL_PRINTF("\n\n");


	//
	// CCM Example
	//

	// Invoke initialization once on the TX!
	if(CCM4LEA_set_init_params(&ccm_TX, K, 128, A, 0, 16) < 0) return -1; // Note that it is the length of T in bytes, not bits

	// Invoke initialization once on the RX!
	if(CCM4LEA_set_init_params(&ccm_RX, K, 128, A, 0, 16) < 0) return -1; // Note that it is the length of T in bytes, not bits


	// TX
	if(CCM4LEA_set_enc_params(&ccm_TX, control_data, 16, N, 12) < 0) return -1;
	if(CCM4LEA_enc(&ccm_TX) < 0) return -1;

	memcpy(payload, ccm_TX.T, 16);
	memcpy(payload+16, ccm_TX.CC, 16);


	// RX
	if(CCM4LEA_set_dec_params(&ccm_RX, payload+16, 16, N, 12, payload) < 0) return -1;
	if(CCM4LEA_dec(&ccm_RX) < 0) return -1; // Mismatch of T


	// The following is for result verification purposes only.
	CONSOL_PRINTF("CCM [Received  T]: ");
	for(i = 0; i < 16; i++) CONSOL_PRINTF("0x%02x ", payload[i]);
	CONSOL_PRINTF("\n");

	CONSOL_PRINTF("CCM [Generated T]: ");
	for(i = 0; i < 16; i++) CONSOL_PRINTF("0x%02x ", ccm_RX.T[i]);
	CONSOL_PRINTF("\n\n");

	CONSOL_PRINTF("CCM [Decrypted control data]: ");
	for (i = 0; i < gcm_RX.PP_byte_length; i++) CONSOL_PRINTF("0x%02x ", ccm_RX.PP[i]);
	CONSOL_PRINTF("\n\n");

    return 0;
}
// End of dron_example.c
