#ifndef _CALIBRATION_H
#define _CALIBRATION_H

#include <samd21.h>
#include <stdint.h>

#ifndef __packed
#define __packed __attribute__((packed))
#endif

// Fuse values, stored in NVM Software Calibration Area.
typedef struct __packed {
  uint32_t _reserved          : 27;
  uint32_t adc_linearity      : 8;
  uint32_t adc_biascal        : 3;
  uint32_t osc32k_cal         : 7;
  uint32_t usb_transn         : 5;
  uint32_t usb_transp         : 5;
  uint32_t usb_trim           : 3;
  uint32_t dfll48m_coarse_cal : 6;
  uint32_t dfll48m_fine_cal   : 10; // Not mentioned in the datasheet, but it exists
  uint32_t _reserved_         : 22;
  uint32_t _reserved__;
} NvmctrlFuses_t;

#define FUSES ((const NvmctrlFuses_t *) NVMCTRL_OTP4)

#endif