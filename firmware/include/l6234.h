#ifndef L6234_included
#define L6234_included

// driver for ST Microelectronics L6234 three-phase motor driver

#include <stdint.h>

#include <libopencm3/stm32/timer.h>

#include "gpio.h"
#include "timer.h"

typedef const struct L6234_channel {
    enum tim_oc_id ena_id;
    gpio_pin       dir_pin;
} L6234_channel;

// `L6234_periph` describe the L6234 hardware (both chip and board).
typedef const struct L6234_periph {
    timer_periph  *timer;
    L6234_channel  channels[3];
} L6234_periph;

// `L6234_config` describes how an app uses the L6234.
typedef struct L6234_config {
    uint32_t       pwm_frequency;
    uint32_t       speed_resolution;
} L6234_config;

// // `L6234` joins `L6234_periph` (hardware) and `L6234_config` (app).
// //
// // XXX I am not happy with this.  Given an L6234 `lp`, you can reach
// // the associated `timer_periph` via `lp->timer->periph` or
// // `lp->periph->timer`.  The app has to know how to assemble all this.
// //
// // Possible solutions:
// //  - Live with it.
// //  - Create global timer struct in l6234.c (hard when it's immutable).
// //  - Make a constructor function of some sort.
// typedef const struct L6234 {
//     L6234_periph  *periph;
//     L6234_config  *config;
// } L6234;

extern void init_L6234(L6234_periph *, L6234_config *);

extern void L6234_set_speed(L6234_periph *, int32_t speed);

extern void L6234_handle_timer_interrupt(L6234_periph *);

extern void L6234_handle_sw_interrupt(L6234_periph *);

#endif /* !L6234_included */
