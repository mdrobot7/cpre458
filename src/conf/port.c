#include "../common/common.h"
#include "conf.h"

#include <samd21.h>

// PORT configuration in common/pinout.h
static uint8_t porta_funcs[32] = PORTA_PINFUNCS;
static uint8_t portb_funcs[32] = PORTB_PINFUNCS;

// Autoconfigures PORT based on common/pinout.h.
void _conf_port() {
  // PORTA
  for (int i = 0; i < 32; i++) {
    switch (porta_funcs[i]) {
      case PORT_PMUX_PMUXE_A_Val:
      case PORT_PMUX_PMUXE_B_Val:
      case PORT_PMUX_PMUXE_C_Val:
      case PORT_PMUX_PMUXE_D_Val:
      case PORT_PMUX_PMUXE_E_Val:
      case PORT_PMUX_PMUXE_F_Val:
      case PORT_PMUX_PMUXE_G_Val:
      case PORT_PMUX_PMUXE_H_Val:
        if ((i % 2) == 0) {
          PORTA->PMUX[i / 2].bit.PMUXE = porta_funcs[i];
        } else {
          PORTA->PMUX[i / 2].bit.PMUXO = porta_funcs[i];
        }
        PORTA->PINCFG[i].reg = PORT_PINCFG_PMUXEN;
        break;
      case PINFUNC_INPUT: // Default config
        PORTA->PINCFG[i].reg = PORT_PINCFG_INEN;
        break;
      case PINFUNC_INPUT_PULLUP:
        PORTA->PINCFG[i].reg = PORT_PINCFG_PULLEN | PORT_PINCFG_INEN;
        PORTA->OUTSET.reg    = (1u << i);
        break;
      case PINFUNC_INPUT_PULLDOWN:
        PORTA->PINCFG[i].reg = PORT_PINCFG_PULLEN | PORT_PINCFG_INEN;
        PORTA->OUTCLR.reg    = (1u << i);
        break;
      case PINFUNC_OUTPUT:
        PORTA->DIRSET.reg = (1u << i);
        break;
      default:
        break;
    }
  }

  // PORTB
  for (int i = 0; i < 32; i++) {
    switch (portb_funcs[i]) {
      case PORT_PMUX_PMUXE_A_Val:
      case PORT_PMUX_PMUXE_B_Val:
      case PORT_PMUX_PMUXE_C_Val:
      case PORT_PMUX_PMUXE_D_Val:
      case PORT_PMUX_PMUXE_E_Val:
      case PORT_PMUX_PMUXE_F_Val:
      case PORT_PMUX_PMUXE_G_Val:
      case PORT_PMUX_PMUXE_H_Val:
        if ((i % 2) == 0) {
          PORTB->PMUX[i / 2].bit.PMUXE = portb_funcs[i];
        } else {
          PORTB->PMUX[i / 2].bit.PMUXO = portb_funcs[i];
        }
        PORTB->PINCFG[i].reg = PORT_PINCFG_PMUXEN;
        break;
      case PINFUNC_INPUT: // Default config
        PORTB->PINCFG[i].reg = PORT_PINCFG_INEN;
        break;
      case PINFUNC_INPUT_PULLUP:
        PORTB->PINCFG[i].reg = PORT_PINCFG_PULLEN | PORT_PINCFG_INEN;
        PORTB->OUTSET.reg    = (1u << i);
        break;
      case PINFUNC_INPUT_PULLDOWN:
        PORTB->PINCFG[i].reg = PORT_PINCFG_PULLEN | PORT_PINCFG_INEN;
        PORTB->OUTCLR.reg    = (1u << i);
        break;
      case PINFUNC_OUTPUT:
        PORTB->DIRSET.reg = (1u << i);
        break;
      default:
        break;
    }
  }
}