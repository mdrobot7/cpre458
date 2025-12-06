#include "scheduler.h"

#include "../common/common.h"
#include "../platform/platform.h"

#include <math.h>

/************************************
 * EXTERN VARIABLES
 ************************************/

/************************************
 * PRIVATE MACROS AND TYPEDEFS
 ************************************/

#define SCHEDULER_ERROR_UNSCHEDULABLE (1)
#define SCHEDULER_ERROR_MK_CONFLICT   (2)

/************************************
 * STATIC VARIABLES
 ************************************/

static uint32_t period_lcm;

static volatile Task_t * priv_tasks;
static int priv_num_tasks;
static int32_t active_task = TASK_NONE;
static uint32_t active_sp  = 0;

/************************************
 * STATIC FUNCTIONS
 ************************************/

static bool is_schedulable(Task_t * tasks, int num_tasks) {
  // I'm lazy, floats it is
  float sum = 0.0;
  for (int i = 0; i < num_tasks; i++) {
    sum += (float) (tasks[i].c * tasks[i].m) / (tasks[i].p * tasks[i].k);
  }
  float thresh = num_tasks * (powf(2.0, 1.0 / num_tasks) - 1.0);
  return sum <= thresh;
}

static bool make_mo_pattern(Task_t * tasks, int num_tasks) {
  for (int i = 0; i < num_tasks; i++) {
    if (tasks[i].mo_pattern == TASK_MO_UNCONFIGURED) {
      tasks[i].mo_pattern = (1 << tasks[i].k) - 1;
      for (int j = i; j < num_tasks; j++) {
        int num_duplicates = 0;
        if (tasks[i].m == tasks[j].m && tasks[i].k == tasks[j].k) {
          // Make sure the patterns don't match, bit rotate left by num_duplicates
          num_duplicates++;
          if (num_duplicates >= 8) {
            return false;
          }
          uint32_t temp = (tasks[i].mo_pattern << num_duplicates);
          temp |= (temp >> tasks[i].k);  // Fill in the trailing 0s
          temp &= (1 << tasks[i].k) - 1; // Mask off the final solution
          tasks[j].mo_pattern = temp & 0xFF;
        }
      }
    }
  }
  return true;
}

static void find_lcm(Task_t * tasks, int num_tasks) {
  // Thanks stack overflow https://stackoverflow.com/questions/185781/finding-the-lcm-of-a-range-of-numbers
  UNUSED(num_tasks);
  period_lcm = tasks[0].p;
  for (int i = 1; i < 10; i++) {
    uint32_t gcd = period_lcm;
    uint32_t tmp = tasks[i].p;
    while (tmp != 0) {
      uint32_t temp_gcd = tmp;
      tmp               = gcd % tmp;
      gcd               = temp_gcd;
    }
    period_lcm = (period_lcm * tasks[i].p) / gcd;
  }
}

static int __noinline mk_firm(Task_t * tasks, int num_tasks) {
  // RMS priority for mandatory runs (smaller period is higher priority),
  // FIFO for optional runs. Force noinline so stack is independent from
  // systick handler.

  // Find the highest priority task that refreshed.
  // TODO: this is RMS, change to MK firm
  uint32_t smallest_period = 0xFFFFFFFF;
  int smallest_period_idx  = num_tasks;
  for (int i = 0; i < num_tasks; i++) {
    tasks[i].sch.timer++;
    if (tasks[i].sch.timer == tasks[i].p) {
      // Task is ready
      tasks[i].sch.timer       = 0;
      tasks[i].sch.task_missed = tasks[i].sch.task_missed || !tasks[i].sch.task_finished;

      if (tasks[i].p < smallest_period) {
        smallest_period     = tasks[i].p;
        smallest_period_idx = i;
      }
    }
  }

  if (smallest_period_idx == num_tasks) {
    // No tasks refreshed, resume what we were doing
    return active_task;
  } else {
    // Preempt the running task
    return smallest_period_idx;
  }
}

/************************************
 * GLOBAL FUNCTIONS
 ************************************/

int scheduler_init(volatile Task_t * tasks, int num_tasks) {
  if (!is_schedulable(tasks, num_tasks)) {
    return SCHEDULER_ERROR_UNSCHEDULABLE;
  }
  if (!make_mo_pattern(tasks, num_tasks)) {
    return SCHEDULER_ERROR_MK_CONFLICT;
  }
  find_lcm(tasks, num_tasks);

  for (int i = 0; i < num_tasks; i++) {
    tasks[i].sch.sp = tasks[i].initial_sp;
  }

  priv_tasks     = tasks;
  priv_num_tasks = num_tasks;

  // It would be nice to make the schedule here, but I likely
  // don't have the RAM to spare.... on the fly it is.

  // Setup systick (platform-specific, could be abstracted better)
  SysTick_Config(480000); // 10ms period
  return 0;
}

__always_inline void scheduler_run() {
  // NOTE: DON'T USE STACK in this function! The stack pointer value is important!
  platform_push_all_regs();

  // Stash the stack pointer from the interrupted process
  platform_active_sp_to_var(active_sp);

  register int task_to_run = mk_firm(priv_tasks, priv_num_tasks);

  if (task_to_run == TASK_NONE || task_to_run == active_task) {
    // Return and resume as normal, either to the task that was interrupted
    // or to an infinite WFI loop at the end of a task.
    return;
  }

  // If we want to return somewhere other than the interrupted process,
  // it's a matter of Indiana-Jones-swapping the IRQ stack frame and then
  // exiting as normal.
  priv_tasks[active_task].sch.sp = active_sp;
  platform_active_var_to_sp(priv_tasks[task_to_run].sch.sp);

  if (priv_tasks[task_to_run].sch.task_started) {
    // Restore previous core state
    platform_pop_all_regs();
  } else {
    platform_setup_task(priv_tasks[task_to_run].entry_point);
  }
}
