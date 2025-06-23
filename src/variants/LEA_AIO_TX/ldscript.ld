/*
**  Abstract    : Linker script for STM32F479IIHX Device with
**                2048KByte FLASH, 320KByte RAM
*/

/* Specify the memory areas */
MEMORY
{
  RAM_CODE (rx)   : ORIGIN = 0x20000000, LENGTH = 1K
  RAM_DATA (xrw)  : ORIGIN = 0x20000000 + LENGTH(RAM_CODE), LENGTH = 320K - LENGTH(RAM_CODE)
  CCMRAM (rw)     : ORIGIN = 0x10000000, LENGTH = 64K
  FLASH (rx)      : ORIGIN = 0x08000000, LENGTH = 2048K
}

INCLUDE "variants/ldscript_gen.ld"
