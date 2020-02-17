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
static int16_t amplitude = 32767;

static void init_phase_masks(L6234_channel chan[3])
{
    uint32_t pin_a = chan[0].dir_pin.gp_pin;
    uint32_t pin_b = chan[1].dir_pin.gp_pin;
    uint32_t pin_c = chan[2].dir_pin.gp_pin;

    phase_positive[0] = pin_a | pin_b;
    phase_positive[1] = pin_a;
    phase_positive[2] = pin_a         | pin_c;
    phase_positive[3] =                 pin_c;
    phase_positive[4] =         pin_b | pin_c;
    phase_positive[5] =         pin_b;
}

void L6234_set_amplitude(L6234_periph *lpp, int16_t a)
{
    lpp = lpp;                  // -Wunused-parameter
    amplitude = a;
}

void L6234_set_position(L6234_periph *lpp, int32_t position)
{
    lpp = lpp;                  // -Wunused-parameter
    muphase = position;
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

// We program the direction inputs (INA, INB and INC) through the
// GPIO interface.  So long as all three are on the same port, we
// can do it with a single register read-modify-write.
static uint32_t dir_gpio_port;
static uint32_t dir_pin_mask;

void init_L6234(L6234_periph *lpp, L6234_config *cfg)
{
    static bool been_here;
    assert(!been_here && "Only one L6234 supported");
    been_here = true;
    timer_periph *tpp = lpp->timer;
    L6234_channel *lpc = lpp->channels;

    dir_gpio_port = lpp->channels[0].dir_pin.gp_port;
    for (size_t i = 0; i < 3; i++) {
        const gpio_pin *pin = &lpc[i].dir_pin;
        gpio_init_pin(pin);
        assert(pin->gp_port == dir_gpio_port);
        dir_pin_mask |= pin->gp_pin;
    }
    init_phase_masks(lpp->channels);
    nvic_enable_irq(TARGET_ADVANCED_TIMER_UP_IRQ);
    enum tim_oc_id ena_a_id = lpc[0].ena_id;
    enum tim_oc_id ena_b_id = lpc[1].ena_id;
    enum tim_oc_id ena_c_id = lpc[2].ena_id;
    timer_config tim_cfg = {
        .pwm_frequency = cfg->pwm_frequency,
        .enable_outputs = 1 << ena_a_id | 1 << ena_b_id | 1 << ena_c_id,
    };
    init_timer(tpp, &tim_cfg);
    timer_enable_pwm(tpp, ena_a_id);
    timer_enable_pwm(tpp, ena_b_id);
    timer_enable_pwm(tpp, ena_c_id);
    timer_enable_irq(tpp->base, TIM_DIER_UIE);
}

__attribute__((optimize("O3")))
void L6234_handle_timer_interrupt(L6234_periph *lpp)
{
    L6234_channel *lpc = lpp->channels;
    timer_periph *tpp = lpp->timer;

    TARGET_trigger_sw_interrupt();

    if (pwm_update.pending) {
        timer_set_pulse_width(tpp, lpc[0].ena_id, pwm_update.width_a);
        timer_set_pulse_width(tpp, lpc[1].ena_id, pwm_update.width_b);
        timer_set_pulse_width(tpp, lpc[2].ena_id, pwm_update.width_c);
        if (pwm_update.phase_changed) {
            uint32_t odr = GPIO_ODR(dir_gpio_port);
            odr &= ~dir_pin_mask;
            odr |= pwm_update.positive_signals;
            GPIO_ODR(dir_gpio_port) = odr;
        }
        pwm_update.pending = false;
    }
}

// faster on MCU with no divide instruction
__attribute__((always_inline))
static uint16_t div6(uint16_t n)
{
    return (uint32_t)n * 43691 >> 18;
}

__attribute__((optimize("O0")))
void L6234_handle_sw_interrupt(L6234_periph *lpp)
{
    if (pwm_update.pending)
        return;

    // muphase += 102;
    // if (muphase >= (6 << CIRCLE_BITS))
    //     muphase -= (6 << CIRCLE_BITS);
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
    assert(amplitude >= 0); // XXX negative amplitude not implemented yet
    int32_t period   = timer_period(lpp->timer);
    int32_t scale    = (amplitude * period) >> 15;

    int32_t  angle_a = div6(muphase + 0 * 1024);
    uint16_t width_a = (abs(sini16(angle_a)) * (scale - 2)) >> 15;
    int32_t  angle_b = div6(muphase + 2 * 1024);
    uint16_t width_b = (abs(sini16(angle_b)) * (scale - 2)) >> 15;
    int32_t  angle_c = div6(muphase + 4 * 1024);
    uint16_t width_c = (abs(sini16(angle_c)) * (scale - 2)) >> 15;

    pwm_update.width_a = width_a;
    pwm_update.width_b = width_b;
    pwm_update.width_c = width_c;
    pwm_update.pending = true;
}
