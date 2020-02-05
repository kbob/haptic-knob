#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>     // XXX

// #include "led.h"
#include "printf.h"
#include "systick.h"
#include "timer.h"
#include "usart.h"
#include TARGET_H

const int USART_BAUD = 115200;

const timer_config timer_cfg = {};

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

static void handle_systick(uint32_t millis)
{
    static uint32_t next_time;
    if (millis >= next_time) {
        // LED_toggle();
        int16_t idx = millis % CIRCLE_SUBDIVISION;
        int32_t s = sini16(idx) + 4095;
        s = (s * abs(s)) >> 16;
        uint16_t v = s >= 0 ? s * 1200 / 32767 : 0;
        timer_set_oc_value(TARGET_timer.base, TIM_OC4, v);
        next_time += 1;
    }
}

int main(void)
{
    rcc_clock_setup_in_hsi_out_48mhz();
    init_systick(rcc_ahb_frequency);
    // init_LED();     // XXX remove
    init_USART(USART_BAUD);
    init_timer(&timer_cfg);
    printf("rcc_ahb_frequency = %lu\n", rcc_ahb_frequency);
    printf("rcc_apb1_frequency = %lu\n", rcc_apb1_frequency);

    init_sin_table();

    register_systick_handler(handle_systick);

    int n = 0;
    while (1) {

        // int pn = n == 3 ? 1 << 31 : n;
        // printf("Hello %d 0x%x %f\n", pn, pn, pn / 3.0);
        printf("CR1 = 0x%lx\n", TIM1_CR1);
        printf("CR2 = 0x%lx\n", TIM1_CR2);
        printf("CCMR2 = 0x%lx\n", TIM1_CCMR2);
        printf("CCR4 = %lu\n", TIM1_CCR4);
        printf("\n");
        n++;
        if (n == 19)
            n = 0x7FFFFFF8;


        for (int i = 0; i < 20000000; i++)
            __asm__ ( "nop" );
    }
}

void __assert_func(const char *file, int line, const char *fn, const char *msg)
{
    printf("Assertion failed: %s, function %s, file %s, line %d\n",
           msg, fn, file, line);
    abort();
}
