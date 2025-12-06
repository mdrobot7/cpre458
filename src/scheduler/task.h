#ifndef _TASK_H
#define _TASK_H

#include "../common/common.h"
#include "../platform/platform.h"

#define TASK_NONE            (-1L)
#define TASK_MO_UNCONFIGURED (0)

#define TASK_SCH_INIT_VAL (0)

#define TASK_CLEANUP(task_struct)                         \
  task_struct.sch.task_finished = true;                   \
  task_struct.sch.sp            = task_struct.initial_sp; \
  while (1) {                                             \
    platform_wfi();                                       \
  }


// Size: 16
typedef struct __packed {
  uint32_t sp;
  uint32_t timer;
  uint32_t optional_task_time;
  uint8_t task_started;
  uint8_t task_finished;
  uint8_t task_missed;
  uint8_t _reserved;
} SchedulerData_t;

// Size: 36
typedef struct __packed {
  uint32_t entry_point;
  uint32_t initial_sp;

  uint32_t c; // Computation time
  uint32_t p; // Period
  uint8_t m;  // Must complete m out of every k runs
  uint8_t k;
  uint8_t mo_pattern; // Mandatory/optional pattern: 1 = M, 0 = O. Making the
                      // user make it is easier than convincing a computer.
  uint8_t _reserved;

  SchedulerData_t sch; // Scheduler-private, initialize to 0
} Task_t;

#endif