#include "timer.h"

#include <assert.h>

#include <libopencm3/stm32/rcc.h>

void init_timer(const timer_periph *tpp, timer_config *cfg)
{
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

    uint32_t period = rcc_apb1_frequency / cfg->pwm_frequency;
    assert(period <= 65536);
    timer_set_period(tim, period - 1);

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

uint32_t timer_period(timer_periph *tpp)
{
    return TIM_ARR(tpp->base) + 1;
}

void timer_force_output_high(timer_periph *tpp, enum tim_oc_id oc)
{
    timer_set_oc_mode(tpp->base, oc, TIM_OCM_FORCE_HIGH);
}

void timer_force_output_low(timer_periph *tpp, enum tim_oc_id oc)
{
    timer_set_oc_mode(tpp->base, oc, TIM_OCM_FORCE_LOW);
}

void timer_enable_pwm(timer_periph *tpp, enum tim_oc_id oc)
{
    timer_set_oc_mode(tpp->base, oc, TIM_OCM_PWM1);
}

void timer_set_pulse_width(timer_periph *tpp, enum tim_oc_id oc, uint16_t width)
{
    timer_set_oc_value(tpp->base, oc, width);
}
