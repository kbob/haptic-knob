#include "usart.h"

#include <limits.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>

#include TARGET_H

static uint32_t default_USART;

void init_USART(const USART *up)
{
    const USART_periph *upp = up->periph;
    const uint32_t usart = upp->base;
    if (!default_USART)
        default_USART = usart;

    rcc_periph_clock_enable(upp->clock);

    gpio_init_pin(&upp->tx);
    gpio_init_pin(&upp->rx);

    // Use default mode of 8 N 1, no flow control
    usart_set_baudrate(usart, up->baud);
    usart_set_mode(usart, USART_MODE_TX_RX);
    usart_enable(usart);
}

void USART_putchar(int c)
{
    if (c == '\n')
        USART_putchar('\r');
    usart_send_blocking(default_USART, c);
}

void USART_putstr(const char *s)
{
    while (*s)
        USART_putchar(*s++);
}

void USART_puts(const char *s)
{
    USART_putstr(s);
    USART_putchar('\n');
}

void USART_putdec(int n)
{
    if (n < 0) {
        if (n == INT_MIN)
            USART_putstr("-2147483648");
        else {
            USART_putchar('-');
            USART_putdec(-n);
        }
        return;
    }
    else {
        if (n >= 10) {
            USART_putdec(n / 10);
            n %= 10;
        }
        USART_putchar('0' + n);
    }
}

void USART_puthex(unsigned n)
{
    if (n >= 0x10) {
        USART_puthex(n >> 4);
        n &= 0xF;
    }
    USART_putchar((n >= 10 ? 'a' - 10 : '0') + n);
}

void USART_putbin(unsigned n)
{
    if (n >= 0b10) {
        USART_putbin(n >> 1);
        n &= 0b1;
    }
    USART_putchar('0' | n);
}
