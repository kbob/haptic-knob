#include "dda.h"

#include <assert.h>

void init_DDA(DDA_state *dda, int32_t dx, int32_t dy, int32_t y0)
{
    assert(dx > 0);
    int32_t q = dy / dx;
    dy -= q * dx;
    if (dy < 0) {
        dy += dx;
        q--;
    }
    dda->dx = dx;
    dda->dy = dy;
    dda->y = y0;
    dda->yi = q;
    dda->D = 2 * dy - dx;
}

int32_t DDA_next(DDA_state *dda)
{
    int32_t y = dda->y;
    if (dda->D > 0) {
        dda->y++;
        dda->D -= 2 * dda->dx;
    }
    dda->y += dda->yi;
    dda->D += 2 * dda->dy;
    return y;
}
