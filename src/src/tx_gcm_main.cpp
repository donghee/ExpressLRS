#include <Arduino.h>
#include "targets.h"
#include "SX1280Driver.h"
#include "common.h"
#include "FHSS.h"

// SX1280Driver Radio;

SX12XX_Radio_Number_t transmittingRadio = Radio.GetLastSuccessfulPacketRadio();

uint8_t testdata[8] = {7, 6, 5, 4, 3, 2, 1, 0};

void ICACHE_RAM_ATTR TXdoneCallback()
{
  //    Serial.println("TXdoneCallback");
    delayMicroseconds(100);
    Radio.TXnb(testdata, sizeof(testdata), transmittingRadio);
}

bool ICACHE_RAM_ATTR RXdoneCallback(SX12xxDriverCommon::rx_status const status)
{
  //    Serial.println("RXdoneCallback");
    for (int i = 0; i < 8; i++)
    {
        Serial.print(Radio.RXdataBuffer[i], HEX);
        Serial.print(",");
    }
    Serial.println("");
    //Radio.RXnb();
    return true;
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Begin SX1280 testing...");

    Radio.Begin();
    //    Radio.Config(SX1280_LORA_BW_0800, SX1280_LORA_SF6, SX1280_LORA_CR_4_7, 2420000000, SX1280_PREAMBLE_LENGTH_32_BITS);
    Radio.Config(SX1280_LORA_BW_0800, SX1280_LORA_SF6, SX1280_LORA_CR_LI_4_8, 0xba1b91, 12, true, 8, 20000, 0, 0, 0);
    // Radio.Config(SX1280_LORA_BW_0800, SX1280_LORA_SF8, SX1280_LORA_CR_LI_4_8, 2420000000, 12, false, 8, 20000, 0, 0, 0);
    Radio.TXdoneCallback = &TXdoneCallback;
    Radio.RXdoneCallback = &RXdoneCallback;
    Radio.SetFrequencyHz(2420000000, transmittingRadio);
    //    Radio.RXnb();

    Radio.TXnb(testdata, sizeof(testdata), transmittingRadio);
}

void loop()
{
    // Serial.println("about to TX");
  //    Radio.TXnb(testdata, sizeof(testdata), transmittingRadio);
    //delay(1000);

    // Serial.println("about to RX");
    //Radio.RXnb();
    delay(100);
    //delay(random(50,200));
    //delay(100);
    //    yield();
}
