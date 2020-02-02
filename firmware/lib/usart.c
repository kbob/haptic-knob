// #define _GNU_SOURCE 1

#include "usart.h"

#include <limits.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>

#include "gpio.h"
#include TARGET_H

#define USART USART1

void init_USART(int baud)
{
    rcc_periph_clock_enable(RCC_USART1);

    gpio_init_pins(TARGET_usart_gpios, TARGET_usart_gpio_count);
    usart_set_baudrate(USART, baud);
    usart_set_databits(USART, 8);
    usart_set_stopbits(USART, USART_STOPBITS_1);
    usart_set_mode(USART, USART_MODE_TX_RX);
    usart_set_parity(USART, USART_PARITY_NONE);
    usart_set_flow_control(USART, USART_FLOWCONTROL_NONE);
    usart_enable(USART);
    USART1_CR1 |= USART_CR1_TE;
}

void USART_putchar(int c)
{
    if (c == '\n')
        USART_putchar('\r');
    usart_send_blocking(USART, c);
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

#if 0

#include <stdio.h>

static ssize_t USART_write(void *cookie, const char *buf, size_t size)
{
    cookie = cookie;            // -Wunused-parameter
    for (size_t i = 0; i < size; i++) {
        char c = buf[i];
        if (c == '\n')
            usart_send_blocking(USART, (uint16_t)'\r');
        usart_send_blocking(USART, (uint16_t)c);
    }
    return size;
}

void USART_init_stdio(void)
{
    cookie_io_functions_t input_fns = {
        .read  = NULL,
        .write = NULL,
        .seek  = NULL,
        .close = NULL,
    };
    cookie_io_functions_t output_fns = {
        .read  = NULL,
        .write = USART_write,
        .seek  = NULL,
        .close = NULL,
    };
    stdin  = fopencookie(NULL, "r", input_fns);
    stdout = fopencookie(NULL, "w", output_fns);
    stderr = fopencookie(NULL, "w", output_fns);
    // setlinebuf(stdout);
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
}

#endif
