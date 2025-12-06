#include "tasks.h"

#include "../common/common.h"

// Static task set
volatile Task_t tasks[] = {
  {
    .entry_point = (uint32_t) (&task_blinky0),
    .initial_sp  = 0xC00,
    .c           = 3,
    .p           = 10,
    .m           = 2,
    .k           = 3,
    .mo_pattern  = TASK_MO_UNCONFIGURED,
    .sch         = { TASK_SCH_INIT_VAL },
  },
  {
    .entry_point = (uint32_t) (&task_blinky1),
    .initial_sp  = 0xB00,
    .c           = 3,
    .p           = 10,
    .m           = 2,
    .k           = 3,
    .mo_pattern  = TASK_MO_UNCONFIGURED,
    .sch         = { TASK_SCH_INIT_VAL },
  },
};

const int num_tasks = ARRAY_SIZE(tasks);