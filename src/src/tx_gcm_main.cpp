#include <Arduino.h>
#include "targets.h"
#include "SX1280Driver.h"
#include "common.h"
#include "FHSS.h"
#include "gcm.h"
#include "rsa.h"
#include "tx_handshake.h"

#define assert(c) if (!(c)) __BKPT()
#define DATA_SIZE 32

SX12XX_Radio_Number_t transmittingRadio = Radio.GetLastSuccessfulPacketRadio();
GCM lea;
TxHandshakeClass TxHandshake;

WORD_ALIGNED_ATTR OTA_Packet_s otaPkt = {0};

void ICACHE_RAM_ATTR TXdoneCallback()
{
  if (!TxHandshake.done())
    TxHandshake.TXdoneCallback();
}

bool ICACHE_RAM_ATTR RXdoneCallback(SX12xxDriverCommon::rx_status const status)
{
  if (!TxHandshake.done())
    TxHandshake.RXdoneCallback(status);
  return true;
}

void rsa_test()
{
    int ret = 1;
    const char *pers = "rsa_genkey";
    unsigned char pubkey[1024] = {0};
    size_t pubkey_len = 0;

    RSA rsa;

    // generate public key
    ret = rsa.generate_key(pers, strlen(pers));
    if ( ret != 0 )
        DBGLN("FAILED GENERATE KEY");
    ret = rsa.export_pubkey(pubkey, &pubkey_len);
    if ( ret != 0 )
        DBGLN("FAILED EXPORT PUBKEY");

    // encrypt using generated public key
    unsigned char plaintext_pub[1024] = {0};
    unsigned char ciphertext_pub[512] = {0};
    plaintext_pub[0] = 0x04; plaintext_pub[1] = 0x03; plaintext_pub[2] = 0x02;
    plaintext_pub[3] = 0x01; plaintext_pub[4] = 0x00;
    ret = rsa.encrypt(pubkey, pubkey_len, plaintext_pub, 5, ciphertext_pub);
    if( ret != 0 )
        DBGLN("FAILED ENCRYPT");

    // encrypt using original key
    unsigned char plaintext[1024] = {0};
    unsigned char ciphertext[512] = {0};
    plaintext[0] = 0x00; plaintext[1] = 0x01; plaintext[2] = 0x02;
    plaintext[2] = 0x03; plaintext[4] = 0x04;
    ret = rsa.encrypt(plaintext, 5, ciphertext);
    if( ret != 0 )
        DBGLN("FAILED ENCRYPT");

    unsigned char decryptedtext[1024] = {0};
    size_t i;

    // decrypt using original key
    ret = rsa.decrypt(ciphertext, decryptedtext, &i, 1024);
    assert(memcmp(plaintext, decryptedtext, 5) == 0);
    ret = rsa.decrypt(ciphertext_pub, decryptedtext, &i, 1024);
    if( ret != 0 )
        DBGLN("FAILED DECRYPT");
    assert(memcmp(plaintext_pub, decryptedtext, 5) == 0);
}

void lea_test()
{
  uint8_t ciphertext[DATA_SIZE] = { 0 };

  otaPkt.std.type = 0;
  otaPkt.std.crcHigh = 0;
  otaPkt.std.rc.ch.raw[0] = 1;
  otaPkt.std.rc.ch.raw[1] = 2;
  otaPkt.std.rc.ch.raw[2] = 3;
  otaPkt.std.rc.ch.raw[3] = 4;
  otaPkt.std.rc.ch.raw[4] = 5;
  otaPkt.std.rc.switches = 6;
  otaPkt.std.rc.ch4 = 0;
  otaPkt.std.crcLow = 7;

  if (lea.encrypt(&otaPkt, ciphertext, 32) != 0) {
    DBGLN("LEA GCM encrypt error");
    return;
  }

  Radio.TXnb(ciphertext, sizeof(ciphertext), transmittingRadio);
}

void setup()
{
  pinMode(GPIO_PIN_LED, OUTPUT);
  digitalWrite(GPIO_PIN_LED, HIGH);

  Serial.begin(115200);
  Serial.println("Begin SX1280 testing...");

  lea.init();

  Radio.Begin();
  Radio.Config(SX1280_LORA_BW_0800, SX1280_LORA_SF6, SX1280_LORA_CR_LI_4_8, 0xba1b91, 12, true, DATA_SIZE, 20000, 0, 0, 0);
  Radio.TXdoneCallback = &TXdoneCallback;
  Radio.RXdoneCallback = &RXdoneCallback;
  Radio.SetFrequencyHz(2420000000, transmittingRadio);

  //rsa_test();
  //lea_test();

  // memset(pubkey, 0, 1024);
  // for(int i = 0; i < 256; i++)
  //   pubkey[i] = i;

  TxHandshake.init();
}

uint8_t K[16] = {0};  uint8_t A[16] = {0};  uint8_t N[12] = {0};
size_t K_len, A_len, N_len;

void loop()
{
  if (!TxHandshake.done()) {
    TxHandshake.do_handle();
  } else {
    TxHandshake.get_lea_key(K, K_len, A, A_len, N, N_len);
    __BKPT();
  }
}
