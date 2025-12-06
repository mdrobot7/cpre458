#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include "../common/common.h"
#include "task.h"

int scheduler_init(volatile Task_t * tasks, int num_tasks);

// Run in SysTick handler
void scheduler_run();

#endif