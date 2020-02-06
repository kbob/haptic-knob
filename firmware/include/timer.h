#ifndef TIMER_included
#define TIMER_included

#include <stdbool.h>
#include <stdint.h>

#include "gpio.h"

typedef struct timer {
    uint32_t        base;
    uint32_t        clock;
    uint32_t        LED_oc_id;
    const gpio_pin *gpios;
    uint32_t        gpio_count;
} timer;

typedef struct timer_config {
    uint32_t        period;
    bool            enable_motor;
    bool            enable_LED;
} timer_config;

extern void init_timer(const timer_config *);

extern void timer_set_LED_duty(uint16_t);

#endif /* !TIMER_included */
