#pragma once

#include "device.h"
extern "C"
{
#include "ccm4lea.h"
}

#define USE_LEA
#define OTA8_LEA_PACKET_SIZE (OTA8_PACKET_SIZE + 3) // NEEDS TO BE MULTIPLE OF 16
#define OTA4_LEA_PACKET_SIZE (OTA4_PACKET_SIZE + 8) // LEA unit block size is 128 bits
