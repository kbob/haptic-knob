#include "math.h"

#include <math.h>
#include <stddef.h>

static int16_t sin_table[QUADRANT_SUBDIVISION + 1];
static const size_t sin_table_size = (&sin_table)[1] - sin_table;

void init_sin_table(void)
{
    for (size_t i = 0; i < sin_table_size; i++) {
        float x = i * (2 * M_PI / CIRCLE_SUBDIVISION);
        sin_table[i] = sinf(x) * INT16_MAX;
    }
}

int16_t sini16(int n)
{
    n &= CIRCLE_SUBDIVISION - 1;
    int quadrant = n >> QUADRANT_BITS;
    int index = n & (QUADRANT_SUBDIVISION - 1);
    int sign = +1;
    if (quadrant & 1) {
        // 2nd and 4th quadrants: reverse index.
        index = QUADRANT_SUBDIVISION - index;
    }
    if (quadrant & 2) {
        // 3rd and 4th quadrants: negative.
        sign = -1;
    }
    return sign * sin_table[index];
}
