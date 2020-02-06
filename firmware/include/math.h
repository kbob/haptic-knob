#ifndef MATH_included
#define MATH_included

#include <stdint.h>

// Instead of degrees or radians, divide the circle into
// CIRCLE_SUBDIVISION parts.  CIRCLE_SUBDIVISION is a power
// of two so wraparound is done by masking off high order bits.

#define CIRCLE_BITS          10
// #define CIRCLE_BITS       4  // for debugging
#define QUADRANT_BITS        (CIRCLE_BITS - 2)
#define CIRCLE_SUBDIVISION   (1 << CIRCLE_BITS)
#define QUADRANT_SUBDIVISION (1 << QUADRANT_BITS)

extern void init_sin_table(void);

extern int16_t sini16(int n);

#endif /* !MATH_included */
