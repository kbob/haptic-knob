#include "l6234.h"

#include <assert.h>
#include <stdbool.h>

#include <libopencm3/cm3/nvic.h>
                                //
#include "printf.h"             // XXX
#include TARGET_H

// `pwm_update` passes data from SW interrupt to timer interrupt.
//
// Timer ISR may read all fields when `pending` is true,
// must clear `pending`.
//
// Software ISR may read/write any fields when `pending` is false,
// must write all fields before setting `pending`.
static struct {
    volatile bool pending;
    bool          phase_changed;
    uint32_t      width_a;
    uint32_t      width_b;
    uint32_t      width_c;
    uint32_t      positive_signals;
} pwm_update;

static uint32_t gpio_port;
static uint32_t gpio_signal_mask;

void init_L6234(L6234 *lp)
{
    static bool been_here;
    assert(!been_here && "Only one L6234 supported");
    been_here = true;
    // timer t = {
    //     .periph = lp->periph->timer,
    //     .config = &lp->config->timer,
    // };
    timer *tp = lp->timer;

    uint32_t first_port = lp->periph->channels[0].pin.gp_port;
    for (size_t i = 0; i < 3; i++) {
        const gpio_pin *pin = &lp->periph->channels[i].pin;
        gpio_init_pin(pin);
        assert(pin->gp_port == first_port);
        gpio_signal_mask |= pin->gp_pin;
    }
    nvic_enable_irq(TARGET_ADVANCED_TIMER_UP_IRQ);
    init_timer(tp);
    timer_enable_pwm(tp, TIM_OC1);
    timer_enable_pwm(tp, TIM_OC2);
    timer_enable_pwm(tp, TIM_OC3);
    uint32_t period = timer_period(tp);
    printf("timer period = %lu\n", period);
    timer_enable_irq(tp->periph->base, TIM_DIER_UIE);
}

void L6234_handle_timer_interrupt(L6234 *lp)
{
    L6234_channel *lpc = lp->periph->channels;
    timer *tp = lp->timer;

    TARGET_trigger_sw_interrupt();

    if (pwm_update.pending) {
        timer_set_pulse_width(tp, lpc[0].id, pwm_update.width_a);
        timer_set_pulse_width(tp, lpc[1].id, pwm_update.width_b);
        timer_set_pulse_width(tp, lpc[2].id, pwm_update.width_c);
        if (pwm_update.phase_changed) {
            uint32_t odr = GPIO_ODR(gpio_port);
            odr &= ~gpio_signal_mask;
            odr |= pwm_update.positive_signals;
            GPIO_ODR(gpio_port) = odr;
        }
        pwm_update.pending = false;
    }
}
