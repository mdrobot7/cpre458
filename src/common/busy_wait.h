#ifndef _BUSY_WAIT_H
#define _BUSY_WAIT_H

#include <stdbool.h>

/**
 * Waits for the specified number of clock cycles in
 * a busy loop. Accurate to a few clock cycles, useful for short
 * one-time waits because of a device's startup time or just
 * making things work while debugging.
 *
 * Check units if time seems to be incorrect. It's written in
 * assembly to ensure timing consistency. Ensure it's compiled with
 * -mthumb if you get asm errors.
 *
 * Make sure we assign to a low register (=l), r0-r7, here. SUBS only
 * supports r0-r7, the Rd and Rn fields are only 3 bits wide, so make
 * sure the assembler always uses valid registers. Larger C ASM syntax
 * reference here https://gcc.gnu.org/onlinedocs/gcc/Machine-Constraints.html
 *
 * GCC needs the .syntax tags before and after this snippet. It struggles
 * with Thumb1 vs Thumb2 syntax, which causes problems with "subs Rd, Rn, #imm".
 * Clang handles it fine. Sigh.
 */
static inline void busy_wait_ms(const unsigned long cpufreq_khz, const unsigned long ms) {
  unsigned long clocks = (cpufreq_khz * ms) / 4;
  __asm__ volatile(
    ".syntax unified\n"
    "label%=: SUBS %0, %0, #1\n"
    "\tNOP\n"
    "\tBNE label%=\n"
    ".syntax divided"
    : "=l"(clocks)
    : "0"(clocks));
}

static inline void busy_wait_us(const unsigned long cpufreq_mhz, const unsigned long us)
  __attribute__((alias("busy_wait_ms")));

#define BUSY_POLL(cpufreq_khz, timeout_ms, interval_ms, timeout_ptr, cond) \
  {                                                                        \
    unsigned long __timeout = 0;                                           \
    while (!(cond) && __timeout < (timeout_ms)) {                          \
      busy_wait_ms(cpufreq_khz, interval_ms);                              \
      __timeout += interval_ms;                                            \
    }                                                                      \
    *(timeout_ptr) = (__timeout >= (timeout_ms));                          \
  }

#endif