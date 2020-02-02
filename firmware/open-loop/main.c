#include <libopencm3/stm32/rcc.h>

#include "led.h"
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

int main(void)
{
    rcc_clock_setup_in_hsi_out_48mhz();
    init_systick(rcc_ahb_frequency);
    init_LED();
    init_USART(USART_BAUD);

    next_time = system_millis;
    register_systick_handler(handle_systick);

    int n = 0;
    while (1) {

        int pn = n == 3 ? 1 << 31 : n;
        USART_putstr("Hello ");
        USART_putdec(pn);
        USART_putstr(" 0x");
        USART_puthex(pn);
        USART_putstr(" 0b");
        USART_putbin(pn);
        USART_putchar('\n');
        n++;
        if (n == 15)
            n = 0x7FFFFFF8;


        for (int i = 0; i < 4000000; i++)
            __asm__ ( "nop" );
    }
}
