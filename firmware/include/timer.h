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

typedef struct timer_oc {       // timer output channel
    enum tim_oc_id  id: 8;
    bool            is_inverted;
    gpio_pin        gpio;
} timer_oc;

// `timer_periph` describes the timer hardware (both chip and board),
// independent of how an app uses it.
typedef struct timer_periph {
    uint32_t          base;
    uint32_t          clock;
    const timer_oc   *out_channels;
    size_t            out_channel_count;
} timer_periph;

// `timer` describes how an app uses a timer.  PWM frequency, outputs
// in use...
typedef struct timer {
    const timer_periph *periph;
    uint32_t            pwm_freq;
    timer_output_bits   enable_outputs;
} timer;

extern void init_timer(const timer *);
extern uint16_t timer_period(const timer *);

extern void timer_force_output_high(const timer *, enum tim_oc_id);
extern void timer_force_output_low(const timer *, enum tim_oc_id);
extern void timer_enable_pwm(const timer *, enum tim_oc_id);

extern void timer_set_pwm_duty(const timer *, enum tim_oc_id, uint16_t duty);

#endif /* !TIMER_included */
