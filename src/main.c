#include "common/common.h"
#include "conf/conf.h"
#include "scheduler/scheduler.h"
#include "scheduler/task.h"
#include "tasks/tasks.h"

#include <samd21.h>

/************************************
 * STATIC VARIABLES
 ************************************/

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
  asm("nop");
  volatile int error = scheduler_init(tasks, num_tasks);
  UNUSED(error);

  while (1) {}
}
