#ifndef _COMMON_H
#define _COMMON_H

#include "busy_wait.h"
#include "calibration.h"
#include "limits.h"
#include "pinout.h"

#include <samd21.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef __packed
#define __packed __attribute__((packed))
#endif

#define PORTA (&PORT->Group[0])
#define PORTB (&PORT->Group[1])

/**
 * Define min and max, preventing double-evaluation of a and b.
 * See https://stackoverflow.com/a/3437484
 */

#define MAX(a, b) \
  ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define MIN(a, b) \
  ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

// Clamp value to within the range [lower, upper], saturating at
// the bounds if necessary. Previously named LIMIT
#define CLAMP(val, lower, upper) \
  (({ __typeof__ (val) _val = (val); \
       __typeof__ (lower) _lower = (lower); \
       __typeof__ (upper) _upper = (upper); \
     (MIN(MAX(_val, _lower), _upper)); }))

// Optimizes beautifully to 4 instructions with -O3 on Cortex M0+
#define ABS(val) \
  (((int32_t) (val) > 0) ? (int32_t) (val) : -(int32_t) (val))

#define UNUSED(x) ((void) x)

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

// Conditional with hysteresis
// Usage: if (ON_HYSTERESIS(...)) { do_on_state_things(); } else { do_off_state_things(); }
#define ON_HYSTERESIS(val, state_true, on_thresh, off_thresh) \
  ((val > on_thresh && !state_true) || ((val > off_thresh) && state_true))

#endif