/*
**  Abstract    : Linker script for STM32F405RGx Device with
**                1024KByte FLASH, 128KByte RAM
*/

/* Specify the memory areas */
MEMORY
{
  RAM_CODE (rx)   : ORIGIN = 0x20000000, LENGTH = 1K
  RAM_DATA (xrw)  : ORIGIN = 0x20000000 + LENGTH(RAM_CODE), LENGTH = 128K - LENGTH(RAM_CODE)
  CCMRAM (rw)     : ORIGIN = 0x10000000, LENGTH = 64K
  FLASH (rx)      : ORIGIN = 0x08000000, LENGTH = 1024K
}

INCLUDE "variants/ldscript_gen.ld"
