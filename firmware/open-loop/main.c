#include <assert.h>
#include <stdlib.h>

#include <libopencm3/stm32/rcc.h>

#include "math.h"
#include "printf.h"
#include "regdump.h"
#include "systick.h"
#include "usart.h"
#include TARGET_H

static const USART console_USART = {
    .periph         = &TARGET_USART,
    .baud           = 115200,
};

static const timer motor_timer = {
    .periph         = &TARGET_advanced_timer,
    .pwm_freq       = 40000,
    .enable_outputs = TOB_OC1N | TOB_OC2N | TOB_OC3N,
};

static void handle_systick(uint32_t millis)
{
    millis = millis;            // -Wunused-parameter
}

static void control_gpio(enum tim_oc_id oc)
{
    const timer_periph *tpp = motor_timer.periph;
    assert(oc < tpp->out_channel_count);
    const timer_oc *tocp = &tpp->out_channels[oc];
    const gpio_pin *tocgp = &tocp->gpio;
    gpio_pin g = {
        .gp_port = tocgp->gp_port,
        .gp_pin  = tocgp->gp_pin,
        .gp_mode = GPIO_MODE_OUTPUT,
    };
    gpio_init_pin(&g);
}

// We generate 3 sine waves for the motor's 3 poles.  They are 120
// degrees apart.  We subdivide the circle into six phases -- each
// phase starts where one of the sines crosses zero: so at 0,npi/3,
// 2pi/3, pi, 4pi/3, and 5pi/3.
//
// `phase` goes from 0 to 5.  `muphase` (microphase) goes from 0
// to (CIRCLE_SUBDIVISION * 6) - 1.
//
// `phase` can be calculated by dividing `muphase` by
// CIRCLE_SUBDIVISION, and angle can be calculated by diving `muphase`
// by 6.
//
// `phase_polarity` maps `phase` onto the subset of waveforms that
// are positive in that phase.
static uint8_t phase;
static uint32_t muphase;

static const uint32_t phase_polarity[6] = {
        GPIO8 | GPIO9,
        GPIO8,
        GPIO8         | GPIO10,
                        GPIO10,
                GPIO9 | GPIO10,
                GPIO9,
};

volatile uint32_t tim_counter;
volatile uint32_t sw_counter;
volatile uint32_t up_counter;

// `pwm_update` passes data from SW interrupt to timer interrupt.
//
// Timer ISR may read all fields when `pending`is true,
// must clear `pending`.
//
// Software ISR may read/write any fields when `pending` is false,
// must write all fields before setting `pending`.
struct {
    volatile bool pending;
    bool          phase_changed;
    uint32_t      width_a;
    uint32_t      width_b;
    uint32_t      width_c;
    uint32_t      positive_signals;
} pwm_update;

// Compiling this function "-O3" makes it slower.  Reason unknown.
__attribute__((optimize("O0")))
extern void TARGET_advanced_timer_up_isr(void)
{
    uint32_t tim = TARGET_advanced_timer.base;
    timer_clear_flag(tim, TIM_SR_UIF);
    tim_counter++;
    assert(!timer_get_flag(tim, TIM_SR_UIF));

    TARGET_trigger_sw_interrupt();

    if (pwm_update.pending) {
        timer_set_pulse_width(&motor_timer, TIM_OC1, pwm_update.width_a);
        timer_set_pulse_width(&motor_timer, TIM_OC2, pwm_update.width_b);
        timer_set_pulse_width(&motor_timer, TIM_OC3, pwm_update.width_c);
        if (pwm_update.phase_changed) {
            uint32_t odr = GPIOA_ODR;
            odr &= ~(GPIO8 | GPIO9 | GPIO10);
            odr |= pwm_update.positive_signals;
            GPIOA_ODR = odr;
        }
        pwm_update.pending = false;
    }

    assert(!timer_get_flag(tim, TIM_SR_UIF));
}

__attribute__((optimize("O3")))
extern void TARGET_sw_isr(void)
{
    exti_reset_request(TARGET_SWINT_EXTI);
    sw_counter++;
    if (pwm_update.pending) {
        up_counter++;
        return;
    }

    muphase += 102;
    if (muphase >= (6 << CIRCLE_BITS))
        muphase -= (6 << CIRCLE_BITS);
    uint8_t nphase = muphase >> CIRCLE_BITS;

    if (phase == nphase) {
        pwm_update.phase_changed = false;
    } else {
        phase = nphase;
        pwm_update.phase_changed = true;
        pwm_update.positive_signals = phase_polarity[nphase];
    }

    // XXX calc once
    // XXX add amplitude parameter
    uint32_t period  = timer_period(&motor_timer);
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

    // // If the update happened, we are too slow.
    assert(!timer_get_flag(TARGET_advanced_timer.base, TIM_SR_UIF));
}

int main(void)
{
    rcc_clock_setup_in_hse_8mhz_out_48mhz();
    init_USART(&console_USART);
    printf("\n\n\n\n\n");
    printf("rcc_ahb_frequency = %lu\n", rcc_ahb_frequency);
    printf("rcc_apb1_frequency = %lu\n", rcc_apb1_frequency);

    init_sin_table();

    // Timer update has highest priority, then systick, then SW interrupt.
    nvic_set_priority(TARGET_ADVANCED_TIMER_UP_IRQ, 0x00);
    nvic_set_priority(NVIC_SYSTICK_IRQ, 0x40);
    nvic_set_priority(TARGET_SWINT_IRQ, 0xC0);

    // enable software interrupt.
    nvic_enable_irq(TARGET_SWINT_IRQ);
    exti_enable_request(TARGET_SWINT_EXTI);

    // enable systick
    init_systick(rcc_ahb_frequency);
    register_systick_handler(handle_systick);

    control_gpio(TIM_OC1);
    control_gpio(TIM_OC2);
    control_gpio(TIM_OC3);

    nvic_enable_irq(TARGET_ADVANCED_TIMER_UP_IRQ);
    init_timer(&motor_timer);
    timer_enable_pwm(&motor_timer, TIM_OC1);
    timer_enable_pwm(&motor_timer, TIM_OC2);
    timer_enable_pwm(&motor_timer, TIM_OC3);
    uint32_t period = timer_period(&motor_timer);
    printf("timer period = %lu\n", period);
    timer_enable_irq(motor_timer.periph->base, TIM_DIER_UIE);

    uint32_t next_time = system_millis;

    while (1) {

        if (system_millis >= next_time) {
            next_time += 100;
            rewind_advanced_timer_registers();
            dump_advanced_timer_registers(TARGET_advanced_timer.base);
        }
    }
}
