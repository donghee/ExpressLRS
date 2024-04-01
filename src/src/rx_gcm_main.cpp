#include <Arduino.h>
#include "targets.h"
#include "SX1280Driver.h"
#include "common.h"
#include "FHSS.h"
#include "gcm.h"
#include "rsa.h"

#define assert(c) if (!(c)) __BKPT()

#define DATA_SIZE 32

SX12XX_Radio_Number_t transmittingRadio = Radio.GetLastSuccessfulPacketRadio();
GCM lea_gcm;

// rsa
unsigned char pubkey[1024] = {0};
size_t pubkey_len = 0;
int pubkey_packageIndex = 0;
size_t packageSize = 8;

// handshake
#include <stubborn_sender.h>
StubbornSender sender;
uint8_t leakey_packageIndex;
uint8_t data[8];
bool confirmValue = true;

enum handshake_state_t {
  HANDSHAKE_INIT,
  HANDSHAKE_WAIT_HELLO,
  HANDSHAKE_WAIT_RSA_PUBKEY,
  HANDSHAKE_SEND_LEA_KEY,
  HANDSHAKE_DONE
};

handshake_state_t handshake_state = HANDSHAKE_INIT;

void ICACHE_RAM_ATTR TXdoneCallback()
{
    // Serial.println("TXdoneCallback1");
}

bool ICACHE_RAM_ATTR RXdoneCallback(SX12xxDriverCommon::rx_status const status)
{
    if (Radio.RXdataBuffer[0] == 'h' && Radio.RXdataBuffer[1] == 'e' &&
        Radio.RXdataBuffer[2] == 'l' && Radio.RXdataBuffer[3] == 'l' &&
        Radio.RXdataBuffer[4] == 'o' && handshake_state == HANDSHAKE_WAIT_HELLO) {
        digitalWrite(GPIO_PIN_LED, !digitalRead(GPIO_PIN_LED));
        handshake_state = HANDSHAKE_WAIT_RSA_PUBKEY;
        pubkey_packageIndex = 0;
        pubkey_len = 0;
    } else if (handshake_state == HANDSHAKE_WAIT_RSA_PUBKEY) {
        if (pubkey_packageIndex >= 32) { // 256 bytes
            handshake_state = HANDSHAKE_DONE;
            // Radio.TXnb((uint8_t*)"ack", 3, transmittingRadio);
            return true;
        }
        memcpy(pubkey + (pubkey_packageIndex * 8), Radio.RXdataBuffer, 8);
        pubkey_packageIndex++;
        pubkey_len += 8;
    }

    Radio.RXnb();
    return true;
}

void rsa_encrypt_test()
{
    int ret = 1;
    const char *pers = "rsa_genkey";

    unsigned char plaintext_pub[1024] = {0};
    unsigned char ciphertext_pub[512] = {0};

    // generate plaintext for test
    uint8_t K[16] = {0x14, 0x87, 0x0B, 0x99, 0x92, 0xEA, 0x89, 0x67, 0x8A, 0x1D, 0xDF, 0xD6, 0x30, 0x91, 0x8D, 0xF0};
    uint8_t A[16] = {0};
    uint8_t N[12] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B};

    memcpy(plaintext_pub, K, 16);
    memcpy(plaintext_pub + 16, A, 16);
    memcpy(plaintext_pub + 32, N, 12);

    RSA rsa;
    // generate key for test
    ret = rsa.generate_key(pers, strlen(pers));
    if ( ret != 0 )
        DBGLN("FAILED GENERATE KEY");

    // encrypt using generated public key
    ret = rsa.encrypt(pubkey, pubkey_len, plaintext_pub, ciphertext_pub, 16+16+12);
    if ( ret != 0 ) {
        __BKPT();
        return;
    }

    // decrypt for test
    unsigned char decryptedtext[1024] = {0};
    size_t i;
    ret = rsa.decrypt(ciphertext_pub, decryptedtext, &i, 1024);
    if( ret != 0 )
        DBGLN("FAILED DECRYPT");

    // hanshake_state = HANDSHAKE_SEND_LEA_KEY;
    // Radio.TXnb(ciphertext_pub, 128, transmittingRadio); // size of ciphertext_pub is 128 bytes = pubkey_len / 2
    //
    // while(HANDSHAKE_DONE != handshake_state) {
    //     delay(100);
    // }
    //
    __BKPT();
}

void setup()
{
    lea_gcm.init();

    pinMode(GPIO_PIN_LED, OUTPUT);
    digitalWrite(GPIO_PIN_LED, HIGH);

    Serial.begin(115200);
    Serial.println("Begin SX1280 testing...");

    Radio.Begin();
    Radio.Config(SX1280_LORA_BW_0800, SX1280_LORA_SF6, SX1280_LORA_CR_LI_4_8, 0xba1b91, 12, true, DATA_SIZE, 20000, 0, 0, 0);
    //Radio.Config(SX1280_LORA_BW_0800, SX1280_LORA_SF8, SX1280_LORA_CR_LI_4_8, 2420000000, 12, false, 8, 20000, 0, 0, 0);
    Radio.TXdoneCallback = &TXdoneCallback;
    Radio.RXdoneCallback = &RXdoneCallback;
    Radio.SetFrequencyHz(2420000000, transmittingRadio);

    handshake_state = HANDSHAKE_WAIT_HELLO;
    Radio.RXnb();
}

void loop()
{
    if (handshake_state == HANDSHAKE_DONE) {

      rsa_encrypt_test();
      handshake_state = HANDSHAKE_WAIT_HELLO;
      Radio.RXnb();
    }
}
