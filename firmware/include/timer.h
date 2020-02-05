#ifndef TIMER_included
#define TIMER_included

#include <stdbool.h>
#include <stdint.h>

#include "gpio.h"

typedef struct timer {
    uint32_t        base;
    uint32_t        clock;
    const gpio_pin *gpios;
    uint32_t        gpio_count;
} timer;

typedef struct timer_config {

} timer_config;

extern void init_timer(const timer_config *);

#endif /* !TIMER_included */
