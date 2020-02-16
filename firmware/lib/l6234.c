#include "l6234.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include <libopencm3/cm3/nvic.h>

#include "math.h"
#include TARGET_H

// We generate three sine waves for the motor's three poles.  They are
// 120 degrees apart.  We subdivide the circle into six phases -- each
// phase starts where one of the sines crosses zero: so at 0, pi/3,
// 2pi/3, pi, 4pi/3, and 5pi/3.
//
// `phase` goes from 0 to 5.  `muphase` (microphase) goes from 0
// to (CIRCLE_SUBDIVISION * 6) - 1.
//
// `phase` can be calculated by dividing `muphase` by
// CIRCLE_SUBDIVISION, and angle can be calculated by diving `muphase`
// by 6.
//
// `phase_positive` maps `phase` onto the subset of waveforms that
// are positive in that phase.  It is represented as a mask of GPIO
// pins.
static uint8_t phase;
static uint32_t muphase;

static uint32_t phase_positive[6];

static void init_phase_mask(L6234_channel chan[3])
{
    uint32_t pin_a = chan[0].pin.gp_pin;
    uint32_t pin_b = chan[1].pin.gp_pin;
    uint32_t pin_c = chan[2].pin.gp_pin;

    phase_positive[0] = pin_a | pin_b;
    phase_positive[1] = pin_a;
    phase_positive[2] = pin_a         | pin_c;
    phase_positive[3] =                 pin_c;
    phase_positive[4] =         pin_b | pin_c;
    phase_positive[5] =         pin_b;
}

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

void init_L6234(L6234_periph *lpp, L6234_config *cfg)
{
    static bool been_here;
    assert(!been_here && "Only one L6234 supported");
    been_here = true;
    timer_periph *tpp = lpp->timer;

    gpio_port = lpp->channels[0].pin.gp_port;
    for (size_t i = 0; i < 3; i++) {
        const gpio_pin *pin = &lpp->channels[i].pin;
        gpio_init_pin(pin);
        assert(pin->gp_port == gpio_port);
        gpio_signal_mask |= pin->gp_pin;
    }
    init_phase_mask(lpp->channels);
    nvic_enable_irq(TARGET_ADVANCED_TIMER_UP_IRQ);
    timer_config tim_cfg = {
        .pwm_frequency = cfg->pwm_frequency,
        .enable_outputs = TOB_OC1N | TOB_OC2N | TOB_OC3N,
    };
    init_timer(tpp, &tim_cfg);
    // XXX use channels[].id
    timer_enable_pwm(tpp, TIM_OC1);
    timer_enable_pwm(tpp, TIM_OC2);
    timer_enable_pwm(tpp, TIM_OC3);
    // uint32_t period = timer_period(lpp->timer);
    // printf("timer period = %lu\n", period);
    timer_enable_irq(tpp->base, TIM_DIER_UIE);
    // printf("GPIOA = %08x; gpio_port = %08lx\n", GPIOA, gpio_port);
    // printf("ODR = %p\n", &GPIO_ODR(gpio_port));
}

volatile uint32_t up_counter;
volatile uint32_t sw_counter;

void L6234_handle_timer_interrupt(L6234_periph *lpp)
{
    up_counter++;
    L6234_channel *lpc = lpp->channels;
    timer_periph *tpp = lpp->timer;

    TARGET_trigger_sw_interrupt();

    if (pwm_update.pending) {
        timer_set_pulse_width(tpp, lpc[0].id, pwm_update.width_a);
        timer_set_pulse_width(tpp, lpc[1].id, pwm_update.width_b);
        timer_set_pulse_width(tpp, lpc[2].id, pwm_update.width_c);
        if (pwm_update.phase_changed) {
            uint32_t odr = GPIO_ODR(gpio_port);
            odr &= ~gpio_signal_mask;
            odr |= pwm_update.positive_signals;
            GPIO_ODR(gpio_port) = odr;
        }
        pwm_update.pending = false;
    }
}

void L6234_handle_sw_interrupt(L6234_periph *lpp)
{
    sw_counter++;
    if (pwm_update.pending)
        return;

    muphase += 102;
    if (muphase >= (6 << CIRCLE_BITS))
        muphase -= (6 << CIRCLE_BITS);
    uint8_t nphase = muphase >> CIRCLE_BITS;

    if (phase == nphase) {
        pwm_update.phase_changed = false;
    } else {
        phase = nphase;
        pwm_update.phase_changed = true;
        pwm_update.positive_signals = phase_positive[nphase];
    }

    // XXX calc once
    // XXX add amplitude parameter
    // XXX `x / 6` is equivalent to `x * 43691 >> 18` for uint16_t x.
    uint32_t period  = timer_period(lpp->timer);
    int32_t  angle_a = (muphase + 0 * 1024) / 6;
    uint16_t width_a = (abs(sini16(angle_a)) * (period - 2)) >> 15;
    int32_t  angle_b = (muphase + 2 * 1024) / 6;
    uint16_t width_b = (abs(sini16(angle_b)) * (period - 2)) >> 15;
    int32_t  angle_c = (muphase + 4 * 1024) / 6;
    uint16_t width_c = (abs(sini16(angle_c)) * (period - 2)) >> 15;

    pwm_update.width_a = width_a;
    pwm_update.width_b = width_b;
    pwm_update.width_c = width_c;
    pwm_update.pending = true;
}
