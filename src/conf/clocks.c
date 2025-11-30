#include "../common/common.h"
#include "conf.h"

#include <samd21.h>

// Maximum Clock Frequencies Table: Datasheet 37.6

typedef struct {
  uint8_t source;
  uint8_t divsel; // False: GCLK = GCLK / div. True: GCLK = GCLK / 2^(div + 1)
  uint16_t div;   // GCLK0, 3-8 have 8 bits. GCLK1 has 16 bits. GCLK2 has 5 bits.
} ClockConf_t;

// Configure clock generators. Connect clock source (i.e. FLL, 32k oscillator) and divide down
static ClockConf_t clocks[GCLK_GEN_NUM] = {
  {
    .source = GCLK_GENCTRL_SRC_DFLL48M_Val, // 48 MHz
    .divsel = 0,
    .div    = 1,
  },
  {
    .source = GCLK_GENCTRL_SRC_OSC32K_Val, // 32.768 kHz
    .divsel = 0,
    .div    = 1,
  },
  { 0 },
  { 0 },
  { 0 },
  { 0 },
  { 0 },
  { 0 },
  {
    .source = GCLK_GENCTRL_SRC_DPLL96M_Val, // 96 MHz
    .divsel = 0,
    .div    = 1,
  },
};

// Select which clock generator to connect to each peripheral.
#define PERIPHERALS_DISABLE (0xFF)
static uint8_t peripherals[GCLK_NUM] = {
  PERIPHERALS_DISABLE,        /**< \brief (GCLK_CLKCTRL) DFLL48 */
  GCLK_CLKCTRL_GEN_GCLK1_Val, /**< \brief (GCLK_CLKCTRL) FDPLL */
  GCLK_CLKCTRL_GEN_GCLK1_Val, /**< \brief (GCLK_CLKCTRL) FDPLL32K */
  PERIPHERALS_DISABLE,        /**< \brief (GCLK_CLKCTRL) WDT */
  PERIPHERALS_DISABLE,        /**< \brief (GCLK_CLKCTRL) RTC */
  PERIPHERALS_DISABLE,        /**< \brief (GCLK_CLKCTRL) EIC */
  PERIPHERALS_DISABLE,        /**< \brief (GCLK_CLKCTRL) USB */
  PERIPHERALS_DISABLE,        /**< \brief (GCLK_CLKCTRL) EVSYS_0 */
  PERIPHERALS_DISABLE,        /**< \brief (GCLK_CLKCTRL) EVSYS_1 */
  PERIPHERALS_DISABLE,        /**< \brief (GCLK_CLKCTRL) EVSYS_2 */
  PERIPHERALS_DISABLE,        /**< \brief (GCLK_CLKCTRL) EVSYS_3 */
  PERIPHERALS_DISABLE,        /**< \brief (GCLK_CLKCTRL) EVSYS_4 */
  PERIPHERALS_DISABLE,        /**< \brief (GCLK_CLKCTRL) EVSYS_5 */
  PERIPHERALS_DISABLE,        /**< \brief (GCLK_CLKCTRL) EVSYS_6 */
  PERIPHERALS_DISABLE,        /**< \brief (GCLK_CLKCTRL) EVSYS_7 */
  PERIPHERALS_DISABLE,        /**< \brief (GCLK_CLKCTRL) EVSYS_8 */
  PERIPHERALS_DISABLE,        /**< \brief (GCLK_CLKCTRL) EVSYS_9 */
  PERIPHERALS_DISABLE,        /**< \brief (GCLK_CLKCTRL) EVSYS_10 */
  PERIPHERALS_DISABLE,        /**< \brief (GCLK_CLKCTRL) EVSYS_11 */
  PERIPHERALS_DISABLE,        /**< \brief (GCLK_CLKCTRL) SERCOMX_SLOW */
  PERIPHERALS_DISABLE,        /**< \brief (GCLK_CLKCTRL) SERCOM0_CORE */
  PERIPHERALS_DISABLE,        /**< \brief (GCLK_CLKCTRL) SERCOM1_CORE */
  PERIPHERALS_DISABLE,        /**< \brief (GCLK_CLKCTRL) SERCOM2_CORE */
  PERIPHERALS_DISABLE,        /**< \brief (GCLK_CLKCTRL) SERCOM3_CORE */
  PERIPHERALS_DISABLE,        /**< \brief (GCLK_CLKCTRL) SERCOM4_CORE */
  PERIPHERALS_DISABLE,        /**< \brief (GCLK_CLKCTRL) SERCOM5_CORE */
  PERIPHERALS_DISABLE,        /**< \brief (GCLK_CLKCTRL) TCC0_TCC1 */
  PERIPHERALS_DISABLE,        /**< \brief (GCLK_CLKCTRL) TCC2_TC3 */
  PERIPHERALS_DISABLE,        /**< \brief (GCLK_CLKCTRL) TC4_TC5 */
  PERIPHERALS_DISABLE,        /**< \brief (GCLK_CLKCTRL) TC6_TC7 */
  PERIPHERALS_DISABLE,        /**< \brief (GCLK_CLKCTRL) ADC */
  PERIPHERALS_DISABLE,        /**< \brief (GCLK_CLKCTRL) AC_DIG */
  PERIPHERALS_DISABLE,        /**< \brief (GCLK_CLKCTRL) AC_ANA */
  PERIPHERALS_DISABLE,        /**< \brief (GCLK_CLKCTRL) DAC */
  PERIPHERALS_DISABLE,        /**< \brief (GCLK_CLKCTRL) PTC */
  PERIPHERALS_DISABLE,        /**< \brief (GCLK_CLKCTRL) I2S_0 */
  PERIPHERALS_DISABLE,        /**< \brief (GCLK_CLKCTRL) I2S_1 */
};

// Connect APB clocks to each peripheral (everything except APBC is
// enabled by default)
// clang-format off
#define PERIPHERAL_APB (PM_APBCMASK_ADC)
// clang-format on

void _conf_clocks() {
  // Start up the DFLL
  SYSCTRL->DFLLCTRL.reg  = SYSCTRL_DFLLCTRL_ENABLE; // Handle Errata 1.2.1
  SYSCTRL->DFLLVAL.reg   = SYSCTRL_DFLLVAL_COARSE(FUSES->dfll48m_coarse_cal) | SYSCTRL_DFLLVAL_FINE(FUSES->dfll48m_fine_cal);
  NVMCTRL->CTRLB.bit.RWS = NVMCTRL_CTRLB_RWS_DUAL; // Increase read/wait states so flash can keep up at 48MHz
  while (!(SYSCTRL->PCLKSR.bit.DFLLRDY)) {}

  // Start up OSC32k
  SYSCTRL->OSC32K.reg = SYSCTRL_OSC32K_CALIB(FUSES->osc32k_cal) | SYSCTRL_OSC32K_EN32K | SYSCTRL_OSC32K_ENABLE;
  while (!(SYSCTRL->PCLKSR.bit.OSC32KRDY)) {}

  // Configure clock generators
  for (int i = 0; i < GCLK_GEN_NUM; i++) {
    if (clocks[i].source) { // XOSC is source 0, we're never using a crystal
      GCLK->GENDIV.reg  = GCLK_GENDIV_DIV(clocks[i].div) | GCLK_GENDIV_ID(i);
      GCLK->GENCTRL.reg = (clocks[i].divsel ? GCLK_GENCTRL_DIVSEL : 0) | (GCLK_GENCTRL_GENEN) | (GCLK_GENCTRL_SRC(clocks[i].source)) | (GCLK_GENCTRL_ID(i));
    }
  }

  // Connect peripherals to clock generators
  for (int i = 0; i < GCLK_NUM; i++) {
    if (peripherals[i] != PERIPHERALS_DISABLE) {
      GCLK->CLKCTRL.reg = (GCLK_CLKCTRL_CLKEN) | (GCLK_CLKCTRL_GEN(peripherals[i])) | (GCLK_CLKCTRL_ID(i));
    }
  }

  // Enable AHB/APB clocks for peripherals
  // Everything is enabled by default except for most of APBC,
  // so only look there
  PM->APBCMASK.reg = PERIPHERAL_APB;

  // Start up DPLL (96MHz)
  // Reference: 32.768 kHz OSC38k
  // Divider = 96 MHz / 32.768 kHz = 2929.6875 = 2928 + 1 + (11/16). See Datasheet 17.6.8.3
  SYSCTRL->DPLLRATIO.reg = SYSCTRL_DPLLRATIO_LDR(2928) | SYSCTRL_DPLLRATIO_LDRFRAC(11);
  SYSCTRL->DPLLCTRLB.reg = SYSCTRL_DPLLCTRLB_LBYPASS | SYSCTRL_DPLLCTRLB_REFCLK_GCLK; // Handle Errata 1.3.1
  SYSCTRL->DPLLCTRLA.reg = SYSCTRL_DPLLCTRLA_ENABLE;
  while (!SYSCTRL->DPLLSTATUS.bit.CLKRDY) {}
}