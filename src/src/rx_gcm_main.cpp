#include <Arduino.h>
#include "targets.h"
#include "SX1280Driver.h"
#include "common.h"
#include "FHSS.h"
#include "gcm.h"

#define assert(c) if (!(c)) __BKPT()

#define DATA_SIZE 32

SX12XX_Radio_Number_t transmittingRadio = Radio.GetLastSuccessfulPacketRadio();
GCM lea_gcm;

// rsa
unsigned char pubkey[1024] = {0};
size_t pubkey_len = 0;
int packageIndex = 0;
size_t packageSize = 8;

enum handshake_state_t {
  HANDSHAKE_INIT,
  HANDSHAKE_WAIT_HELLO,
  HANDSHAKE_WAIT_RSA_PUBKEY,
  HANDSHAKE_DONE
};

handshake_state_t handshake_state = HANDSHAKE_INIT;

void ICACHE_RAM_ATTR TXdoneCallback()
{
    Serial.println("TXdoneCallback1");
}

bool ICACHE_RAM_ATTR RXdoneCallback(SX12xxDriverCommon::rx_status const status)
{

    // uint8_t plaintext[DATA_SIZE];
    // int ret = 0;
    // ret = lea_gcm.decrypt((OTA_Packet_s *) plaintext, (const uint8_t *) Radio.RXdataBuffer, 32);
    //
    // if (ret == 0)
    //   digitalWrite(GPIO_PIN_LED, !digitalRead(GPIO_PIN_LED));
    // delay(100);

    if (Radio.RXdataBuffer[0] == 'h' && Radio.RXdataBuffer[1] == 'e' &&
        Radio.RXdataBuffer[2] == 'l' && Radio.RXdataBuffer[3] == 'l' &&
        Radio.RXdataBuffer[4] == 'o' && handshake_state == HANDSHAKE_WAIT_HELLO) {
        digitalWrite(GPIO_PIN_LED, !digitalRead(GPIO_PIN_LED));
        handshake_state = HANDSHAKE_WAIT_RSA_PUBKEY;
        packageIndex = 0;
    } else if (handshake_state == HANDSHAKE_WAIT_RSA_PUBKEY) {
        if (packageIndex >= 32) {
            handshake_state = HANDSHAKE_DONE;
            __BKPT();
            memset(pubkey, 0, sizeof(pubkey));
            return true;
        }
        memcpy(pubkey + (packageIndex * 8), Radio.RXdataBuffer, 8);
        packageIndex++;
    }

    Radio.RXnb();
    return true;
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
        handshake_state = HANDSHAKE_WAIT_HELLO;
        Radio.RXnb();
    }
}
