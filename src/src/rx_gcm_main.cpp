#include <Arduino.h>
#include "targets.h"
#include "SX1280Driver.h"
#include "common.h"
#include "FHSS.h"

//SX1280Driver Radio;

SX12XX_Radio_Number_t transmittingRadio = Radio.GetLastSuccessfulPacketRadio();

uint8_t testdata[8] = {0x80};

void ICACHE_RAM_ATTR TXdoneCallback()
{
    Serial.println("TXdoneCallback1");
    //delay(1000);
    //Radio.TXnb(testdata, sizeof(testdata));
}

bool ICACHE_RAM_ATTR RXdoneCallback(SX12xxDriverCommon::rx_status const status)
{
    // Serial.println("RXdoneCallback");
    // for (int i = 0; i < 8; i++)
    // {
    //     Serial.print(Radio.RXdataBuffer[i], HEX);
    //     Serial.print(",");
    // }
    // Serial.println("");
    Radio.RXnb();

    digitalWrite(GPIO_PIN_LED, !digitalRead(GPIO_PIN_LED));
    //    delay(500);

    return true;
}

void setup()
{
    pinMode(GPIO_PIN_LED, OUTPUT);
    digitalWrite(GPIO_PIN_LED, HIGH);

    Serial.begin(115200);
    Serial.println("Begin SX1280 testing...");

    Radio.Begin();
    Radio.Config(SX1280_LORA_BW_0800, SX1280_LORA_SF6, SX1280_LORA_CR_LI_4_8, 0xba1b91, 12, true, 8, 20000, 0, 0, 0);
    //Radio.Config(SX1280_LORA_BW_0800, SX1280_LORA_SF8, SX1280_LORA_CR_LI_4_8, 2420000000, 12, false, 8, 20000, 0, 0, 0);
    Radio.TXdoneCallback = &TXdoneCallback;
    Radio.RXdoneCallback = &RXdoneCallback;
    Radio.SetFrequencyHz(2420000000, transmittingRadio);
    Radio.RXnb();
}



void loop()
{
  //    digitalWrite(GPIO_PIN_LED, !digitalRead(GPIO_PIN_LED));
  //    delay(500);
    //delay(250);
    //Serial.println("about to TX");
    //Radio.TXnb(testdata, 8, transmittingRadio);

    // Serial.println("about to RX");
  //Radio.RXnb();
    // delay(random(50,200));
}
