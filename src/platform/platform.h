#ifndef _PLATFORM_H
#define _PLATFORM_H

#include "../common/common.h"

// Pushes all regsiters to the stack (that aren't
// pushed by ABI or hardware). Must be called first in
// the systick handler.
// Hardware handles r0-r3, r12, r14 (ARMv6m B1.5.6)
inline __always_inline static void platform_push_all_regs() {
  __asm__ volatile(
    "mov r0,r8\n\t"
    "mov r1,r9\n\t"
    "mov r2,r10\n\t"
    "mov r3,r11\n\t"
    "push {r0, r1, r2, r3, r4, r5, r6, r7}"
    :                        // No inputs
    :                        // No outputs
    : "r0", "r1", "r2", "r3" // Mark r0-3 as modified
  );
}

// Pops all registers from the stack (that aren't
// popped by the ABI or hardware). Must be called last
// in the systick handler.
inline __always_inline static void platform_pop_all_regs() {
  __asm__ volatile(
    "pop {r0, r1, r2, r3, r4, r5, r6, r7}\n\t"
    "mov r8, r0\n\t"
    "mov r9, r1\n\t"
    "mov r10, r2\n\t"
    "mov r11, r3"
    :                          // No inputs
    :                          // No outputs
    : "r8", "r9", "r10", "r11" // Mark r8-11 as modified
  );
}

// Set a variable = the currently active process's SP.
// Offset by the size of the exception stack frame
inline __always_inline static void platform_active_sp_to_var(uint32_t var) {
  __asm__ volatile(
    "mov %[out_var], sp\n\t"
    "add %[out_var], sp, #32" // 0x20
    : [out_var] "=r"(var));
}

// Set the SP = variable.
inline __always_inline static void platform_active_var_to_sp(uint32_t var) {
  __asm__ volatile(
    "mov sp, %[in_var]"
    : // No inputs
    : [in_var] "r"(var)
    : "sp" // SP was modified
  );
}

// Sets up the exception stack frame to return to a
// task that hasn't been started yet (and doesn't have an
// existing stack frame).
// Overwrite the exception return address with the entry point
// of our task.
inline __always_inline static void platform_setup_task(uint32_t entry_point) {
  __asm__ volatile(
    "str %[in_entry_point], [sp, #24]" // 0x18
    :                                  // No inputs
    : [in_entry_point] "r"(entry_point));
}

inline __always_inline static void platform_wfi() {
  __asm__ volatile("wfi");
}

#endif