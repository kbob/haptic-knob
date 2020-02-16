#ifndef TIMER_included
#define TIMER_included

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <libopencm3/stm32/timer.h>

#include "gpio.h"

typedef enum timer_output_bits {
    TOB_OC1  = 1 << TIM_OC1,
    TOB_OC1N = 1 << TIM_OC1N,
    TOB_OC2  = 1 << TIM_OC2,
    TOB_OC2N = 1 << TIM_OC2N,
    TOB_OC3  = 1 << TIM_OC3,
    TOB_OC3N = 1 << TIM_OC3N,
    TOB_OC4  = 1 << TIM_OC4,
} timer_output_bits;

// timer output channel
typedef const struct timer_oc { // timer output channel
    enum tim_oc_id    id: 8;
    bool              is_inverted;
    gpio_pin          gpio;
} timer_oc;

// `timer_periph` describes the timer hardware (both chip and board),
// independent of how an app uses it.
typedef const struct timer_periph {
    uint32_t          base;
    uint32_t          clock;
    timer_oc         *out_channels;
    size_t            out_channel_count;
} timer_periph;

// `timer_config` describes how an app uses a timer.  PWM frequency,
// outputs n use...
typedef const struct timer_config {
    uint32_t          pwm_frequency;
    timer_output_bits enable_outputs;
} timer_config;

// // `timer` joins `timer_periph` (hardware) with `timer_config` (app).
// typedef const struct timer {
//     timer_periph     *periph;
//     timer_config     *config;
// } timer;

extern void init_timer(timer_periph *, timer_config *);
extern uint32_t timer_period(timer_periph *);

extern void timer_force_output_high(timer_periph *, enum tim_oc_id);
extern void timer_force_output_low(timer_periph *, enum tim_oc_id);
extern void timer_enable_pwm(timer_periph *, enum tim_oc_id);

extern void timer_set_pulse_width(timer_periph *, enum tim_oc_id, uint16_t wid);

#endif /* !TIMER_included */
