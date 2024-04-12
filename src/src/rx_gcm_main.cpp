#include <Arduino.h>
#include "targets.h"
#include "SX1280Driver.h"
#include "common.h"
#include "FHSS.h"
#include "gcm.h"
#include "rsa.h"
#include "rx_handshake.h"

#define assert(c) if (!(c)) __BKPT()
#define DATA_SIZE 32

SX12XX_Radio_Number_t transmittingRadio = Radio.GetLastSuccessfulPacketRadio();
GCM lea_gcm;
RxHandshakeClass RxHandshake;

void ICACHE_RAM_ATTR TXdoneCallback()
{
  if (!RxHandshake.done())
    RxHandshake.TXdoneCallback();
}

bool ICACHE_RAM_ATTR RXdoneCallback(SX12xxDriverCommon::rx_status const status)
{
  if (!RxHandshake.done())
    RxHandshake.RXdoneCallback(status);
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

    delay(15000);

    RxHandshake.init();
}

void loop()
{
  // if (!RxHandshake.done())
  RxHandshake.do_handle();
}
