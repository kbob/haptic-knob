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


extern void init_L6234(L6234_periph *, L6234_config *);

#define L6234_AMPLITUDE_MIN (-32767)
#define L6234_AMPLITUDE_MAX (+32767)
extern void L6234_set_amplitude(L6234_periph *, int16_t amp);

// Set position in units of 6 * 1024 per circle.
#define L6234_CIRCLE (6 * 1024)
extern void L6234_set_position(L6234_periph *, int32_t pos);

extern void L6234_handle_timer_interrupt(L6234_periph *);

extern void L6234_handle_sw_interrupt(L6234_periph *);

#endif /* !L6234_included */
