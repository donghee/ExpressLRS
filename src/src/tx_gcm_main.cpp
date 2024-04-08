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
GCM lea;

WORD_ALIGNED_ATTR OTA_Packet_s otaPkt = {0};

volatile bool busyTransmitting;

// lea
uint8_t ciphertext[DATA_SIZE] = { 0 };
uint8_t testtext[DATA_SIZE] = { 0 };
unsigned char lea_key[512] = {0};
size_t lea_key_len = 0;
int lea_key_packageIndex = 0;

// rsa
unsigned char pubkey[1024] = {0};
size_t pubkey_len = 0;
int pubkey_msg_num = 0;
RSA rsa;

// handshake
#include <stubborn_sender.h>
StubbornSender sender;
volatile uint8_t packageIndex;
volatile int pubkey_msg_seq = 0;
uint8_t data[8];
volatile bool confirmValue = true;
uint8_t batterySequence[] = {0xEC,10,0x08,0,0,0,0,0,0,0,1,109};
uint32_t pubkey_timeout = 10;
uint32_t lea_key_timeout = 10;

// void ICACHE_RAM_ATTR TXdoneCallback()
// {
//   int ret = 0;
//   //  delayMicroseconds(100);
//   memcpy(testtext, &otaPkt, sizeof(otaPkt));
//
//   ret = lea.encrypt(&otaPkt, ciphertext, 32);
//   if (ret != 0) {
//     DBGLN("LEA GCM encrypt error");
//     return;
//   }
//
//   uint8_t plaintext[DATA_SIZE];
//   ret = lea.decrypt((OTA_Packet_s *) plaintext, (const uint8_t *) ciphertext, 32);
//   if (ret != 0) {
//     DBGLN("LEA GCM decrypt error");
//   }
//
//   digitalWrite(GPIO_PIN_LED, !digitalRead(GPIO_PIN_LED));
//   // Radio.TXnb(ciphertext, sizeof(ciphertext), transmittingRadio);
//
//   Radio.TXnb(pubkey + pubkey_msg_num, 32, transmittingRadio);
//   if (pubkey_msg_num >= 224) {
//     pubkey_msg_num = 0;
//   } else {
//     pubkey_msg_num += 32;
//   }
// }

enum handshake_state_t {
  HANDSHAKE_INIT,
  HANDSHAKE_HELLO,
  HANDSHAKE_SEND_RSA_PUBKEY,
  HANDSHAKE_WAIT_ACK,
  HANDSHAKE_RECV_LEA_KEY,
  HANDSHAKE_DECRYPT_LEA_KEY,
  HANDSHAKE_DONE
};

handshake_state_t handshake_state = HANDSHAKE_INIT;

uint8_t prepareDateForPubkey()
{
    uint8_t packageIndex_;
    packageIndex_ = sender.GetCurrentPayload(data, sizeof(data));
    sender.ConfirmCurrentPayload(confirmValue);
    confirmValue = !confirmValue;
    return packageIndex_;
}

void ICACHE_RAM_ATTR TXdoneCallback()
{
  if (!busyTransmitting)
  {
    return; // Already finished transmission
  }

  busyTransmitting = false;
}

bool ICACHE_RAM_ATTR RXdoneCallback(SX12xxDriverCommon::rx_status const status)
{
  if (memcmp(Radio.RXdataBuffer, "ack", 3) == 0) {
    handshake_state = HANDSHAKE_RECV_LEA_KEY;
    lea_key_packageIndex = 0;
    lea_key_len = 0;
  } else if (handshake_state == HANDSHAKE_RECV_LEA_KEY) {
    if (lea_key_packageIndex == 15) {
      memcpy(lea_key + (lea_key_packageIndex * 8), Radio.RXdataBuffer, 8);
      handshake_state = HANDSHAKE_DECRYPT_LEA_KEY;
      return true;
    }

    memcpy(lea_key + (lea_key_packageIndex * 8), Radio.RXdataBuffer, 8);
    lea_key_packageIndex++;
    lea_key_len += 8;
  }

  Radio.RXnb();
  return true;
}

void rsa_test()
{
    int ret = 1;
    const char *pers = "rsa_genkey";

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
    ret = rsa.encrypt(pubkey, pubkey_len, plaintext_pub, ciphertext_pub, 5);
    if( ret != 0 )
        DBGLN("FAILED ENCRYPT");

    // encrypt using original key
    unsigned char plaintext[1024] = {0};
    unsigned char ciphertext[512] = {0};
    plaintext[0] = 0x00; plaintext[1] = 0x01; plaintext[2] = 0x02;
    plaintext[3] = 0x03; plaintext[4] = 0x04;
    ret = rsa.encrypt(plaintext, ciphertext, 5);
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

    //Radio.TXnb(pubkey, pubkey_len, transmittingRadio); // pubkey_len is 256
    //Radio.TXnb(pubkey, 32, transmittingRadio);
}

void handshake_hello()
{
  busyTransmitting = true;
  Radio.TXnb((uint8_t*)"hello", 5, transmittingRadio);
}

int handshake_send_rsa_pubkey()
{
 //  if (handshake_state == HANDSHAKE_HELLO) {
 //    // start sending pubkey
 //    handshake_state = HANDSHAKE_SEND_RSA_PUBKEY;
 //    sender.setMaxPackageIndex(ELRS4_TELEMETRY_MAX_PACKAGES);
 //    sender.ResetState();
 //    sender.SetDataToTransmit(pubkey, 32);
 //    packageIndex = handshake_send_pubkey();
 //    pubkey_msg_seq = 0;
 // } else if (handshake_state == HANDSHAKE_SEND_RSA_PUBKEY) {
 //    if (pubkey_msg_seq == 8) { // last pubkey msg
 //      handshake_state  = HANDSHAKE_DONE;
 //      pubkey_msg_seq = 0;
 //      return;
 //    }
 //    // send next pubkey package
 //    packageIndex = handshake_send_pubkey();
 //    if (packageIndex == 0) { // at last package of current msg, prepare next 32 bytes pubkey msg
 //      sender.setMaxPackageIndex(ELRS4_TELEMETRY_MAX_PACKAGES);
 //      sender.ResetState();
 //      sender.SetDataToTransmit(pubkey + (32 * (pubkey_msg_seq + 1)), 32);
 //      pubkey_msg_seq++;
 //    }
 //  }
 //
 //  Radio.TXnb(data, sizeof(data), transmittingRadio);

  busyTransmitting = true;

  if (pubkey_msg_seq == 8) { // last pubkey msg
      pubkey_msg_seq = 0;
      return 0; // done sending pubkey
  }

  packageIndex = prepareDateForPubkey();

  if (packageIndex == 0) { // at last package of current msg, prepare next 32 bytes pubkey msg
    sender.setMaxPackageIndex(ELRS4_TELEMETRY_MAX_PACKAGES);
    sender.ResetState();
    sender.SetDataToTransmit(pubkey + (32 * (pubkey_msg_seq + 1)), 32);
    pubkey_msg_seq++;
  }

  Radio.TXnb(data, sizeof(data), transmittingRadio);

  return 1; // continue sending pubkey
}

void handshake_wait_ack()
{
  Radio.RXnb();
}

void handshake_recv_lea_key()
{
}

void lea_test()
{
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

  rsa_test();
  //lea_test();

  // memset(pubkey, 0, 1024);
  // for(int i = 0; i < 256; i++)
  //   pubkey[i] = i;

  handshake_state = HANDSHAKE_INIT;
}

void loop()
{
  int ret = 1;

  // decrypt lea key
  unsigned char decryptedtext[1024] = {0};
  size_t i;

  if (handshake_state == HANDSHAKE_DECRYPT_LEA_KEY) {
    ret = rsa.decrypt(lea_key, decryptedtext, &i, 1024);
    if (ret != 0)
      DBGLN("FAILED DECRYPT");

    if (ret == 0)
      handshake_state = HANDSHAKE_DONE;
    else
      handshake_state = HANDSHAKE_INIT;
  }

  if (handshake_state == HANDSHAKE_DONE) {
    digitalWrite(GPIO_PIN_LED, !digitalRead(GPIO_PIN_LED));
    handshake_state = HANDSHAKE_INIT;
  }

  if (handshake_state == HANDSHAKE_INIT) {
    handshake_state = HANDSHAKE_HELLO;
  }

  if (handshake_state == HANDSHAKE_HELLO) {
    handshake_hello();

    while (busyTransmitting) { }

    busyTransmitting = true;
    handshake_state = HANDSHAKE_SEND_RSA_PUBKEY;
    sender.setMaxPackageIndex(ELRS4_TELEMETRY_MAX_PACKAGES);
    sender.ResetState();
    sender.SetDataToTransmit(pubkey, 32);
    packageIndex = prepareDateForPubkey(); // TODO, add data reference
    pubkey_msg_seq = 0;

    Radio.TXnb(data, sizeof(data), transmittingRadio);
  }

  if (handshake_state == HANDSHAKE_SEND_RSA_PUBKEY) {
    if (!busyTransmitting) {
      ret = handshake_send_rsa_pubkey();
    }
    if (ret == 0) {
      pubkey_timeout = millis() + 10;
      handshake_state = HANDSHAKE_WAIT_ACK;
      Radio.RXnb();
    }
  }

  if (handshake_state == HANDSHAKE_WAIT_ACK) {
    if (millis() > pubkey_timeout) {  // on timeout, restart handshake
      handshake_state = HANDSHAKE_INIT;
    }
  }

  if (handshake_state == HANDSHAKE_RECV_LEA_KEY) {
  }
}
