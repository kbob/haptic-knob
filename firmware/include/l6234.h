#ifndef L6234_included
#define L6234_included

// driver for ST Microelectronics L6234 three-phase motor driver

#include <stdint.h>

#include <libopencm3/stm32/timer.h>

#include "gpio.h"
#include "timer.h"

typedef const struct L6234_channel {
    enum tim_oc_id id;
    gpio_pin       pin;
} L6234_channel;

// `L6234_periph` describe the L6234 hardware (both chip and board).
typedef const struct L6234_periph {
    timer_periph  *timer;
    L6234_channel  channels[3];
} L6234_periph;

// `L6234_config` describes how an app uses the L6234.
typedef struct L6234_config {
    uint32_t       speed_resolution;
} L6234_config;

// `L6234` joins `L6234_periph` (hardware) and `L6234_config` (app).
//
// XXX I am not happy with this.  Given an L6234 `lp`, you can reach
// the associated `timer_periph` via `lp->timer->periph` or
// `lp->periph->timer`.  The app has to know how to assemble all this.
typedef const struct L6234 {
    L6234_periph  *periph;
    L6234_config  *config;
    timer         *timer;
} L6234;

extern void init_L6234(L6234 *);

extern void L6234_set_speed(L6234 *, int32_t speed);

extern void L6234_handle_timer_interrupt(L6234 *);

extern void L6234_handle_sw_interrupt(L6234 *);

#endif /* !L6234_included */
