#ifndef _CRC_H
#define _CRC_H

#include "common.h"

/**
 * @brief Calculate CRC16/CCITT for a data buffer. Uses
 * DMAC's CRC engine.
 *
 * @param data Buffer to calculate for, must be 16-bit aligned
 * @param len Length of buffer in *bytes*, must be a multiple of 2
 * @return CRC16/CCITT checksum value
 *
 */
static inline uint16_t crc16(uint16_t * data, uint32_t len) {
  static bool initialized = false;
  if (!initialized) {
    DMAC->CTRL.bit.CRCENABLE = 0;
    DMAC->CRCCTRL.reg        = (DMAC_CRCCTRL_CRCSRC_IO | DMAC_CRCCTRL_CRCPOLY_CRC16 | DMAC_CRCCTRL_CRCBEATSIZE_HWORD);
    DMAC->CRCCHKSUM.reg      = 0;
    DMAC->CTRL.bit.CRCENABLE = 1u;
    initialized              = true;
  }

  for (uint32_t i = 0; i < (len / 2); i++) {
    DMAC->CRCDATAIN.reg = data[i];
    asm("nop"); // 16 bit CRC takes 2 cycles to calculate, see Errata 1.10.1
  }
  uint16_t crc = (DMAC->CRCCHKSUM.reg & 0xFFFF);
  // Zero out CRCCHKSUM by feeding the previous CRC into it (have to byteswap for endianness here)
  DMAC->CRCDATAIN.reg = ((crc >> 8) | (crc << 8)) & 0xFFFF;

  return crc;
}

#endif