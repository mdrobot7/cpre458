#include "common/common.h"
#include "conf/conf.h"

#include <samd21.h>

/************************************
 * INTERRUPT HANDLERS
 ************************************/

/************************************
 * MAIN
 ************************************/

void main() {
  asm("nop"); // Force debug to stop here
  conf();

  /* INITS */

  while (1) {
    PORTA->OUTTGL.reg = PORT_PA02;
    busy_wait_ms(48000, 250);
  }
}
