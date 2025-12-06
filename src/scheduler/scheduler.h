#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include "../common/common.h"
#include "task.h"

extern volatile int32_t scheduler_active_task;

int scheduler_init(volatile Task_t * tasks, int num_tasks);

int scheduler_run();

#endif