#include <math.h>
#include <stdint.h>

#include <libopencm3/stm32/rcc.h>

#include "led.h"
#include "printf.h"
#include "systick.h"
#include "usart.h"

const int USART_BAUD = 115200;

static uint32_t next_time;

static void handle_systick(uint32_t millis)
{
    if (millis >= next_time) {
        LED_toggle();
        next_time += 500;
    }
}

#define CIRCLE_BITS 10
// #define CIRCLE_BITS 4
#define QUADRANT_BITS (CIRCLE_BITS - 2)
#define CIRCLE_SUBDIVISION (1 << CIRCLE_BITS)
#define QUADRANT_SUBDIVISION (1 << QUADRANT_BITS)
int16_t sin_table[QUADRANT_SUBDIVISION + 1];
const size_t sin_table_size = (&sin_table)[1] - sin_table;

static void init_sin_table(void)
{
    for (size_t i = 0; i < sin_table_size; i++) {
        float x = i * (2 * M_PI / CIRCLE_SUBDIVISION);
        sin_table[i] = sinf(x) * INT16_MAX;
    }
    printf("\n");
}

int16_t sini16(int n);
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

int main(void)
{
    rcc_clock_setup_in_hsi_out_48mhz();
    init_systick(rcc_ahb_frequency);
    init_LED();
    init_USART(USART_BAUD);
    printf("rcc_ahb_frequency = %lu\n", rcc_ahb_frequency);
    printf("rcc_apb1_frequency = %lu\n", rcc_apb1_frequency);

    init_sin_table();

    next_time = system_millis;
    register_systick_handler(handle_systick);

    int n = 0;
    while (1) {

        int pn = n == 3 ? 1 << 31 : n;
        printf("Hello %d 0x%x %f\n", pn, pn, pn / 3.0);
        n++;
        if (n == 19)
            n = 0x7FFFFFF8;


        for (int i = 0; i < 20000000; i++)
            __asm__ ( "nop" );
    }
}
