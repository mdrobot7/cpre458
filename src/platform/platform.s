/*
  SysTick handler procedure (platform-specific).

  Must perform the operations:
  - Save off core state (including exception return address) to
    stack
  - Swap to scheduler's stack
  - Call the scheduler
  - Swap currently running process stack
  - Run the following snippet:
    if (task_to_run != TASK_NONE && task_to_run != active_task) {
      if (active_task != TASK_NONE) {
        tasks[active_task].sch.sp = [sp from core state save]
      }
      sp = tasks[task_to_run].sch.sp

      if (tasks[task_to_run].sch.task_started) {
        Restore core state from stack
      } else {
        Exception return address = tasks[task_to_run].entry_point
      }
    } else {
      Restore core state from stack
    }

  Needs a few C symbols:
  - scheduler_run()
  - scheduler_active_task
  - tasks[]

  Creates one C symbol:
  - Platform_SysTick_Handler
*/

  .syntax unified

  .global Platform_SysTick_Handler

  .section .text
  .thumb_func

/*
  Hardware:
  push {r0, r1, r2, r3, r12, lr, return_addr, xPSR}
  lr = [exception exit branch target]
*/
Platform_SysTick_Handler:
  /*
    Save off core state into stack. Hardware saves certain registers, only
    save the ones not handled by hardware.
  */
  mov r0, r8
  mov r1, r9
  mov r2, r10
  mov r3, r11
  push {r0, r1, r2, r3, r4, r5, r6, r7}
  push {lr} // Stash lr before it's overwritten by bl scheduler_run

  /*
    Save off stack pointer and swap to scheduler's stack
  */
  mov r4, sp
  ldr r1, =_esched_stack
  mov sp, r1

  /*
    Call scheduler, returns task_to_run in r0
  */
  bl scheduler_run

  /*
    Swap back to current task's stack
  */
  mov sp, r4
  pop {r7} // Pop exception's lr and save for later
  mov lr, r7

  /*
    if (task_to_run != TASK_NONE && task_to_run != active_task) {}
  */
  movs r3, #0
  subs r3, #1
  cmp r0, r3
  beq restore_core_state
  ldr r1, =scheduler_active_task
  ldr r2, [r1]
  cmp r0, r2
  beq restore_core_state

  /*
    Stack pointer operations
  */
start_different_task:
  str r0, [r1] // Update the active task
  ldr r1, =tasks

  cmp r2, r3
  beq no_active_task
  movs r3, #36 // sizeof(Task_t)
  muls r3, r2
  adds r3, #20 // offset of Task_t.sch.sp
  add r3, r1
  mov r4, sp
  str r4, [r3]

no_active_task:
  movs r3, #36
  muls r3, r0
  adds r3, #20
  add r3, r1
  ldr r4, [r3]
  mov sp, r4

  /*
    if (tasks[task_to_run].sch.task_started) {}
  */
  adds r3, #13 // &Task_t.sch.sp + 12 = &Task_t.sch.task_started
  ldrb r4, [r3]
  cmp r4, #0
  beq start_new_task
restore_core_state:
  pop {r0, r1, r2, r3, r4, r5, r6, r7}
  mov r8, r0
  mov r9, r1
  mov r10, r2
  mov r11, r3
  b exit
start_new_task:
  subs r3, #33 // &Task_t.sch.task_started - 32 = &Task_t.entry_point
  ldr r5, [r3]
  sub sp, #32 // Allocate the exception stack frame
  ldr r6, =0x01000000 // Force T-bit high
  str r6, [sp, #28] // 0x1C
  str r5, [sp, #24] // 0x18
exit:
  bx lr // Not the actual pc, it's the magic "exception return" pc

/*
  Hardware: pop {r0, r1, r2, r3, r12, lr, return_addr, xPSR}
*/
