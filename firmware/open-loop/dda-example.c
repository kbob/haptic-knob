#include <assert.h>
#include <stdio.h>

// Handle dx < |dy|, dx > |dy|, dy < 0.



void dda(int x0, int x1, int y0, int y1)
{
    int dx = x1 - x0;
    int dy = y1 - y0;
    assert(0 <= dy && dy <= dx);
    int x = x0, y = y0;
    int D = 0;
    D = 2 * dy - dx;
    for (int i = 0; i < dx; i++) {
        printf("(%d, %d)\n", x, y);
        if (D > 0) {
            y++;
            D -= 2 * dx;
        }
        x++;
        D += 2 * dy;
    }
}

void dda2(int dx, int y0, int y1)
{
    int dy = y1 - y0;
    assert(0 <= dy && dy <= dx);
    int D = 2 * dy - dx;
    int y = y0;
    for (int i = 0; i < dx; i++) {
        printf("%d\n", y);
        if (D > 0) {
            y++;
            D -= 2 * dx;
        }
        D += 2 * dy;
    }
}

typedef struct dda_state {
    int dx, dy;
    int y;
    int yi;
    int D;
} dda_state;

void init_dda(dda_state *dda, int dx, int y0, int dy)
{
    assert(dx > 0);
    int q = dy / dx;
    dy -= q * dx;
    if (dy < 0) {
        dy += dx;
        q -= 1;
    }
    dda->dx = dx;
    dda->dy = dy;
    dda->y = y0;
    dda->yi = q;
    dda->D = 2 * dy - dx;
    dda->D = 0;
}

int dda_next(dda_state *dda)
{
    int y = dda->y;
    if (dda->D > 0) {
        dda->y++;
        dda->D -= 2 * dda->dx;
    }
    dda->y += dda->yi;
    dda->D += 2 * dda->dy;
    return y;
}

void dda3(int dx, int y0, int y1)
{
    dda_state dda;
    init_dda(&dda, dx, y0, y1 - y0);
    for (int i = 0; i < dx; i++) {
        printf("%d\n", dda_next(&dda));
    }
}

void print_dda3(int dx, int dy, int n)
{
    dda_state dda;
    init_dda(&dda, dx, 0, dy);
    printf("%d / %d:", dy, dx);
    for (int i = 0; i < n; i++)
        printf(" %d", dda_next(&dda));
    printf(" (q = %d, r = %d)\n", dda.yi, dda.dy);
}
int main()
{
    // dda(1, 21, 3, 8);
    // printf("-----\n");
    // dda2(20, 3, 8);
    // printf("-----\n");
    // dda3(20, 3, 8);
    print_dda3(5, +3, 11);
    print_dda3(3, +5, 6);
    print_dda3(5, -3, 11);
    print_dda3(3, -5, 6);
    // for (int i = -5; i <= +5; i += 10)
    //     for (int j = -3; j <= +3; j += 6)
    //         printf("%d / %d = %d r %d\n", i, j, i / j, i % j);
    return 0;
}
