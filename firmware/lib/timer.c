#include "timer.h"

#include <assert.h>

#include <libopencm3/stm32/rcc.h>

void init_timer(const timer *tp)
{
    const timer_periph *tpp = tp->periph;
    const timer_config *cfg = tp->config;
    uint32_t tim = tpp->base;
    for (size_t i = 0; i < tpp->out_channel_count; i++) {
        assert(i == tpp->out_channels[i].id);
    }

    rcc_periph_clock_enable(tpp->clock);
    for (size_t i = 0; i < tpp->out_channel_count; i++) {
        const timer_oc *op = &tpp->out_channels[i];
        if (cfg->enable_outputs & (1 << op->id))
            gpio_init_pin(&op->gpio);
    }

    timer_set_period(tim, timer_period(tp));

    for (size_t i = 0; i < tpp->out_channel_count; i++) {
        const timer_oc *op = &tpp->out_channels[i];
        if (cfg->enable_outputs & (1 << op->id)) {
            timer_enable_oc_preload(tim, op->id);
            timer_enable_oc_output(tim, op->id);
        }
    }

    timer_enable_break_main_output(tim);
    timer_enable_counter(tim);
}

uint16_t timer_period(const timer *tp)
{
    uint32_t period = rcc_apb1_frequency / tp->config->pwm_freq;
    assert(period < 65536);
    return period;
}

void timer_force_output_high(const timer *tp, enum tim_oc_id oc)
{
    timer_set_oc_mode(tp->periph->base, oc, TIM_OCM_FORCE_HIGH);
}

void timer_force_output_low(const timer *tp, enum tim_oc_id oc)
{
    timer_set_oc_mode(tp->periph->base, oc, TIM_OCM_FORCE_LOW);
}

void timer_enable_pwm(const timer *tp, enum tim_oc_id oc)
{
    timer_set_oc_mode(tp->periph->base, oc, TIM_OCM_PWM1);
}

void timer_set_pulse_width(const timer *tp, enum tim_oc_id oc, uint16_t width)
{
    timer_set_oc_value(tp->periph->base, oc, width);
}
