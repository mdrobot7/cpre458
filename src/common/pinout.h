#ifndef _PINOUT_H
#define _PINOUT_H

#include <samd21.h>

/*
    Pinout for the micro, along with PORT configuration info
    for conf/port.c. Must be here so pin defs can be global.
*/

// Pin functions, so port.c can autoconfigure pins
#define _PINFUNC_MUX           (15) // Max value of PMUX
#define PINFUNC_INPUT          (16)
#define PINFUNC_INPUT_PULLUP   (17)
#define PINFUNC_INPUT_PULLDOWN (18)
#define PINFUNC_OUTPUT         (19)
#define PINFUNC_UNUSED         (255)

// Dev board
// #define PIN_DEBUG_1     (PORT_PA12)
// #define PINFUNC_DEBUG_1 (PINFUNC_OUTPUT)

#define PORTA_PINFUNCS         \
  {                            \
    PINFUNC_UNUSED, /* PA00 */ \
    PINFUNC_UNUSED, /* PA01 */ \
    PINFUNC_OUTPUT, /* PA02 */ \
    PINFUNC_UNUSED, /* PA03 */ \
    PINFUNC_UNUSED, /* PA04 */ \
    PINFUNC_UNUSED, /* PA05 */ \
    PINFUNC_UNUSED, /* PA06 */ \
    PINFUNC_UNUSED, /* PA07 */ \
    PINFUNC_UNUSED, /* PA08 */ \
    PINFUNC_UNUSED, /* PA09 */ \
    PINFUNC_UNUSED, /* PA10 */ \
    PINFUNC_UNUSED, /* PA11 */ \
    PINFUNC_UNUSED, /* PA12 */ \
    PINFUNC_UNUSED, /* PA13 */ \
    PINFUNC_UNUSED, /* PA14 */ \
    PINFUNC_UNUSED, /* PA15 */ \
    PINFUNC_UNUSED, /* PA16 */ \
    PINFUNC_UNUSED, /* PA17 */ \
    PINFUNC_UNUSED, /* PA18 */ \
    PINFUNC_UNUSED, /* PA19 */ \
    PINFUNC_UNUSED, /* PA20 */ \
    PINFUNC_UNUSED, /* PA21 */ \
    PINFUNC_UNUSED, /* PA22 */ \
    PINFUNC_UNUSED, /* PA23 */ \
    PINFUNC_UNUSED, /* PA24 */ \
    PINFUNC_UNUSED, /* PA25 */ \
    PINFUNC_UNUSED, /* PA26 */ \
    PINFUNC_UNUSED, /* PA27 */ \
    PINFUNC_UNUSED, /* PA28 */ \
    PINFUNC_UNUSED, /* PA29 */ \
    PINFUNC_UNUSED, /* PA30 */ \
    PINFUNC_UNUSED, /* PA31 */ \
  }

#define PORTB_PINFUNCS         \
  {                            \
    PINFUNC_UNUSED, /* PB00 */ \
    PINFUNC_UNUSED, /* PB01 */ \
    PINFUNC_UNUSED, /* PB02 */ \
    PINFUNC_UNUSED, /* PB03 */ \
    PINFUNC_UNUSED, /* PB04 */ \
    PINFUNC_UNUSED, /* PB05 */ \
    PINFUNC_UNUSED, /* PB06 */ \
    PINFUNC_UNUSED, /* PB07 */ \
    PINFUNC_UNUSED, /* PB08 */ \
    PINFUNC_UNUSED, /* PB09 */ \
    PINFUNC_UNUSED, /* PB10 */ \
    PINFUNC_UNUSED, /* PB11 */ \
    PINFUNC_UNUSED, /* PB12 */ \
    PINFUNC_UNUSED, /* PB13 */ \
    PINFUNC_UNUSED, /* PB14 */ \
    PINFUNC_UNUSED, /* PB15 */ \
    PINFUNC_UNUSED, /* PB16 */ \
    PINFUNC_UNUSED, /* PB17 */ \
    PINFUNC_UNUSED, /* PB18 */ \
    PINFUNC_UNUSED, /* PB19 */ \
    PINFUNC_UNUSED, /* PB20 */ \
    PINFUNC_UNUSED, /* PB21 */ \
    PINFUNC_UNUSED, /* PB22 */ \
    PINFUNC_UNUSED, /* PB23 */ \
    PINFUNC_UNUSED, /* PB24 */ \
    PINFUNC_UNUSED, /* PB25 */ \
    PINFUNC_UNUSED, /* PB26 */ \
    PINFUNC_UNUSED, /* PB27 */ \
    PINFUNC_UNUSED, /* PB28 */ \
    PINFUNC_UNUSED, /* PB29 */ \
    PINFUNC_UNUSED, /* PB30 */ \
    PINFUNC_UNUSED, /* PB31 */ \
  }

#endif