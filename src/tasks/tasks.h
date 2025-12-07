#ifndef _TASKS_H
#define _TASKS_H

#include "../scheduler/scheduler.h"
#include "../scheduler/task.h"

extern volatile Task_t tasks[];
extern const int num_tasks;

void task_blinky0();
void task_blinky1();
void task_blinky2();

#endif