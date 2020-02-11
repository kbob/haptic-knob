#ifndef USART_included
#define USART_included

#include <stdint.h>

#include "gpio.h"

typedef struct USART_periph {
    uint32_t            base;
    uint32_t            clock;
    gpio_pin            tx, rx;
} USART_periph;

typedef struct USART {
    const USART_periph *periph;
    uint32_t            baud;
} USART;

extern void init_USART(const USART *);

extern void USART_puts(const char *);
extern void USART_putchar(int);

extern void USART_putstr(const char *);
extern void USART_putdec(int);
extern void USART_puthex(unsigned);
extern void USART_putbin(unsigned);

#endif /* !USART_included */
