#include "../common/common.h"
#include "../scheduler/task.h"
#include "tasks.h"

/************************************
 * EXTERN VARIABLES
 ************************************/

/************************************
 * PRIVATE MACROS AND TYPEDEFS
 ************************************/

/************************************
 * STATIC VARIABLES
 ************************************/

/************************************
 * STATIC FUNCTIONS
 ************************************/

/************************************
 * GLOBAL FUNCTIONS
 ************************************/

void task_blinky0() {
  TASK_STARTUP(tasks[0]);

  for (int i = 0; i < 10; i++) {
    PORTA->OUTTGL.reg = PORT_PA02;
    busy_wait_ms(48000, 100); // Artifically lengthen the task
  }
  asm("nop");
  TASK_CLEANUP(tasks[0]);
}

void task_blinky1() {
  TASK_STARTUP(tasks[1]);

  volatile int make_my_stack_bigger[10];

  for (int i = 0; i < 10; i++) {
    PORTA->OUTTGL.reg = PORT_PA03;
    busy_wait_ms(48000, 100); // Artifically lengthen the task
  }
  asm("nop");
  TASK_CLEANUP(tasks[1]);
}