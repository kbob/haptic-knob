#ifndef DDA_included
#define DDA_included

#include <stdint.h>

// DDA - digital differental analyzer.  Generate successive points on
// the line $y = mx + y0$.
//
// Assumes 0 <= dy <= dx.

typedef struct DDA_state {
    int32_t dx, dy;
    int32_t y;
    int32_t yi;
    int32_t D;
} DDA_state;

extern void init_DDA(DDA_state *, int32_t dx, int32_t dy, int32_t y0);

extern int32_t DDA_next(DDA_state *);

#endif /* !DDA_included */
