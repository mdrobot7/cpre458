#ifndef _PLATFORM_H
#define _PLATFORM_H

#include "../common/common.h"

extern void Platform_SysTick_Handler(void);

__always_inline inline static void platform_wfi() {
  __asm__ volatile("wfi");
}

#endif