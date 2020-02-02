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

#define CIRCLE_SUBDIVISION 1024
// #define CIRCLE_SUBDIVISION 16
#define QUADRANT_SUBDIVISION (CIRCLE_SUBDIVISION / 4)
int16_t sin_table[QUADRANT_SUBDIVISION + 1];
const size_t sin_table_size = (&sin_table)[1] - sin_table;

static void init_sin_table(void)
{
    printf("sin_table_size = %u\n", sin_table_size);
    for (size_t i = 0; i < sin_table_size; i++) {
        float x = i * (2 * M_PI / CIRCLE_SUBDIVISION);
        sin_table[i] = sinf(x) * INT16_MAX;
        printf("sin %u: %d\n", i, sin_table[i]);
    }
    printf("\n");
}

int main(void)
{
    rcc_clock_setup_in_hsi_out_48mhz();
    init_systick(rcc_ahb_frequency);
    init_LED();
    init_USART(USART_BAUD);

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
