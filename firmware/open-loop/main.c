#include <assert.h>
#include <stdlib.h>

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>

#include "intr.h"
#include "math.h"
#include "printf.h"
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

//      ;       ;       ;       ;       ;       ;       ;       ;       ;
// We generate 3 sine waves for the motor's 3 poles.  They are 120
// degrees apart.  We divide the circle into (CIRCLE_SUBDIVISION *
// 6
//
static uint8_t phase;
static uint32_t muphase;

static uint32_t commutate(void)
{
    switch (phase) {

    case 0:                     // phase 0 - A, B positive
        // gpio_set(GPIOA, GPIO8 | GPIO10);
        // gpio_clear(GPIOA, GPIO9);
        // break;
        return GPIO8 | GPIO10;

    case 1:
        // gpio_set(GPIOA, GPIO8);
        // gpio_clear(GPIOA, GPIO9 | GPIO10);
        // break;
        return GPIO8;

    case 2:
        // gpio_set(GPIOA, GPIO8 | GPIO9);
        // gpio_clear(GPIOA, GPIO10);
        // break;
        return GPIO8 | GPIO9;

    case 3:
        // gpio_set(GPIOA, GPIO9);
        // gpio_clear(GPIOA, GPIO8 | GPIO10);
        // break;
        return GPIO9;

    case 4:
        // gpio_set(GPIOA, GPIO9 | GPIO10);
        // gpio_clear(GPIOA, GPIO8);
        // break;
        return GPIO9 | GPIO10;

    case 5:
        // gpio_set(GPIOA, GPIO10);
        // gpio_clear(GPIOA, GPIO8 | GPIO9);
        // break;
        return GPIO10;

    default:
        assert(0 && "invalid phase");
    }
}

#include <libopencm3/cm3/systick.h>

volatile uint32_t tim1_sr;
volatile uint32_t tim1_srx;
volatile uint32_t caa, cbb, cr;
volatile uint32_t cc_counter;
volatile uint32_t sw_counter;

static inline uint32_t div6(uint32_t n)
{
    uint32_t t0 = (n << 2) + n;
    uint32_t t1 = (t0 << 8) + (t0 << 4) + t0;
    uint32_t t2 = (t1 << 1) + n;
    return t2 >> 14;
}

// Timer ISR may read all fields when `pending`is true,
// must clear `pending`.
//
// Software ISR may read/write any fields when `pending` is false,
// must write all fields before setting `pending`.
struct {
    volatile bool pending;
    bool          phase_changed;
    uint32_t      duration_a;
    uint32_t      duration_b;
    uint32_t      duration_c;
    uint32_t      positive_signals;
} pwm_update;

extern void tim1_cc_isr(void)
{
    timer_clear_flag(TIM1, TIM_SR_UIF | TIM_SR_CC4IF);
    cc_counter++;
    tim1_sr = TIM1_SR;
    assert(!timer_get_flag(TIM1, TIM_SR_UIF));

    TARGET_trigger_sw_interrupt();

    if (pwm_update.pending) {
        timer_set_pwm_duty(&motor_timer, TIM_OC1, pwm_update.duration_a);
        timer_set_pwm_duty(&motor_timer, TIM_OC2, pwm_update.duration_b);
        timer_set_pwm_duty(&motor_timer, TIM_OC3, pwm_update.duration_c);
        if (pwm_update.phase_changed) {
            uint32_t odr = GPIOA_ODR;
            odr &= ~(GPIO8 | GPIO9 | GPIO10);
            odr |= pwm_update.positive_signals;
            GPIOA_ODR = odr;
        }
        pwm_update.pending = false;
    }

    assert(!timer_get_flag(TIM1, TIM_SR_UIF));
}

extern void TARGET_sw_isr(void)
{
cr = STK_RVR;
uint32_t tmp_caa = STK_CVR;
    exti_reset_request(TARGET_SW_EXTI);
    sw_counter++;
    if (pwm_update.pending)
        return;
    assert(!pwm_update.pending);

    // In one cycle,
    //   phase runs from 0..6.
    //   angle runs from 0..1024.
    //   muphase (microphase) runs from 0..(240 * 1024) = .

    // phase: 0..6
    // angle: 0..1024
    // muphase: 0..(6 * 1024)
    // phase = muphase / 1024
    // angle = muphase / 6
    // phase to 6 in 240 steps
    // muphase to (240 * 1024) in 240 steps; step size 1024
    // angle to 1024 in 240 steps; step size 1024 / 240
    // phase to 6 in 240 steps; step size 1 / 40

    muphase += 102;
    if (muphase >= (6 << CIRCLE_BITS))
        muphase -= (6 << CIRCLE_BITS);
    uint8_t nphase = muphase >> CIRCLE_BITS;

    if (phase != nphase) {
        phase = nphase;
        pwm_update.phase_changed = true;
        pwm_update.positive_signals = commutate();
    } else
        pwm_update.phase_changed = false;

    // XXX calc once
    // XXX add amplitude parameter
    uint32_t period = timer_period(&motor_timer);
    // int32_t angle_a = (muphase - 0 * 1024) / 6;
    int32_t angle_a = div6(muphase - 0 * 1024);
    uint16_t duty_a = (abs(sini16(angle_a)) * (period - 2)) >> 15;
    // timer_set_pwm_duty(&motor_timer, TIM_OC1, duty_a);
    pwm_update.duration_a = duty_a;
    int32_t angle_b = div6(muphase + 4 * 1024);
    uint16_t duty_b = (abs(sini16(angle_b)) * (period - 2)) >> 15;
    pwm_update.duration_b = duty_b;
    // timer_set_pwm_duty(&motor_timer, TIM_OC2, duty_b);
    int32_t angle_c = div6(muphase + 2 * 1024);
    uint16_t duty_c = (abs(sini16(angle_c)) * (period - 2)) >> 15;
    pwm_update.duration_c = duty_c;
    // timer_set_pwm_duty(&motor_timer, TIM_OC3, duty_c);

    pwm_update.pending = true;

    // // If the update happened, we are too slow.
    assert(!timer_get_flag(TIM1, TIM_SR_UIF));
    tim1_srx = TIM1_SR;
uint32_t tmp_cbb = STK_CVR;
    static uint32_t prev_millis;
    if (prev_millis != system_millis) {
        prev_millis = system_millis;

    } else {
        cbb = tmp_cbb;
        caa = tmp_caa;
    }
}

// extern void tim1_brk_up_trg_com_isr(void)
// {
//     assert(0 && "dead code");
//     // // If the update happened, we are too slow.
//     // assert(!(TIM1_SR & TIM_SR_UIF));
//     // assert(!timer_get_flag(TIM1, TIM_SR_UIF));
//     // // assert(!timer_interrupt_source(TIM1, TIM_SR_UIF));
//     // timer_clear_flag(TIM1, TIM_SR_UIF | TIM_SR_CC4IF);
//
//     // In one cycle,
//     //   phase runs from 0..6.
//     //   angle runs from 0..1024.
//     //   muphase (microphase) runs from 0..(240 * 1024) = .
//
//
//     // phase: 0..6
//     // angle: 0..1024
//     // muphase: 0..(6 * 1024)
//     // phase = muphase / 1024
//     // angle = muphase / 6
//     // phase to 6 in 240 steps
//     // muphase to (240 * 1024) in 240 steps; step size 1024
//     // angle to 1024 in 240 steps; step size 1024 / 240
//     // phase to 6 in 240 steps; step size 1 / 40
//     muphase += 1024;
//     if (muphase >= (6 * 1024))
//         muphase -= (6 * 1024);
//     uint8_t nphase = muphase / (1024);
//
//     if (phase != nphase) {
//         phase = nphase;
//         commutate();
//     }
//
//     // XXX calc once
//     // XXX add amplitude parameter
//     uint32_t period = timer_period(&motor_timer);
//
//     int32_t angle_a = (muphase - 0 * 1024) / 6;
//     uint16_t duty_a = (abs(sini16(angle_a)) * (period - 2)) >> 15;
//     timer_set_pwm_duty(&motor_timer, TIM_OC1, duty_a);
//     int32_t angle_b = (muphase + 4 * 1024) / 6;
//     uint16_t duty_b = (abs(sini16(angle_b)) * (period - 2)) >> 15;
//     timer_set_pwm_duty(&motor_timer, TIM_OC2, duty_b);
//     int32_t angle_c = (muphase + 2 * 1024) / 6;
//     uint16_t duty_c = (abs(sini16(angle_c)) * (period - 2)) >> 15;
//     timer_set_pwm_duty(&motor_timer, TIM_OC3, duty_c);
//
//     // If the update happened, we are too slow.
//     // assert(!timer_get_flag(TIM1, TIM_SR_UIF));
//     timer_clear_flag(TIM1, TIM_SR_UIF | TIM_SR_CC4IF);
// }

static void dump_tim1_registers(void)
{
    static bool been_here;
    if (!been_here) {
        been_here = true;
        printf("\n\n\n\n\n");
    }
    printf("\33[A\33[A\33[A\33[A\33[A");
    printf("TIM1 registers:\n");
    printf("    CR1   %4lx  SR    %4lx  CCER  %4lx  RCR   %4lx  CCR4  %4lx\n",
           TIM1_CR1,   TIM1_SR,    TIM1_CCER,  TIM1_RCR,   TIM1_CCR4);
    printf("    CR2   %4lx  EGR   %4lx  CNT   %4lx  CCR1  %4lx  BDTR  %4lx\n",
           TIM1_CR2,   TIM1_EGR,   TIM1_CNT,   TIM1_CCR1,  TIM1_BDTR);
    printf("    SMCR  %4lx  CCMR1 %4lx  PSC   %4lx  CCR2  %4lx  DCR   %4lx\n",
           TIM1_SMCR,  TIM1_CCMR1, TIM1_PSC,   TIM1_CCR2,  TIM1_DCR);
    printf("    DIER  %4lx  CCMR2 %4lx  ARR   %4lx  CCR3  %4lx  DMAR  %4lx\n",
           TIM1_DIER,  TIM1_CCMR2, TIM1_ARR,   TIM1_CCR3,  TIM1_DMAR);
}

int main(void)
{
    rcc_clock_setup_in_hse_8mhz_out_48mhz();
    init_USART(&console_USART);
    printf("\n\n\n\n\n");
    printf("rcc_ahb_frequency = %lu\n", rcc_ahb_frequency);
    printf("rcc_apb1_frequency = %lu\n", rcc_apb1_frequency);

    init_sin_table();
    init_systick(rcc_ahb_frequency);
    register_systick_handler(handle_systick);

    nvic_set_priority(NVIC_EXTI2_3_IRQ, 0xC0);

    nvic_enable_irq(NVIC_EXTI2_3_IRQ);
    // enable exti driver
    exti_enable_request(TARGET_SW_EXTI);

    init_timer(&motor_timer);
    timer_enable_pwm(&motor_timer, TIM_OC1);
    timer_enable_pwm(&motor_timer, TIM_OC2);
    timer_enable_pwm(&motor_timer, TIM_OC3);
    timer_enable_pwm(&motor_timer, TIM_OC4);
    uint32_t period = timer_period(&motor_timer);
    printf("timer period = %lu\n", period);
    timer_set_pwm_duty(&motor_timer, TIM_OC4, 1);
    timer_enable_irq(motor_timer.periph->base, TIM_DIER_CC4IE);
    // timer_enable_irq(motor_timer.periph->base, TIM_DIER_UIE);
#define SCB_SHPR(ipr_id)                MMIO8(SCS_BASE + 0xD18 + (ipr_id))
    printf("systick prio = %d\n", SCB_SHPR((NVIC_SYSTICK_IRQ & 0xF) - 4));
    nvic_set_priority(NVIC_SYSTICK_IRQ, 0x80);
    printf("systick prio = %d\n", SCB_SHPR((NVIC_SYSTICK_IRQ & 0xF) - 4));
    nvic_enable_irq(NVIC_TIM1_CC_IRQ);
    // nvic_enable_irq(NVIC_TIM1_BRK_UP_TRG_COM_IRQ);
    // nvic_enable_irq(NVIC_EXTI2_3_IRQ);

    control_gpio(TIM_OC1);
    control_gpio(TIM_OC2);
    control_gpio(TIM_OC3);

    uint32_t next_time = system_millis;

    while (1) {

        if (system_millis >= next_time) {
            next_time += 1000;
            dump_tim1_registers();
        }
    }
}

void __assert_func(const char *file, int line, const char *fn, const char *msg)
{
    cm_disable_interrupts();
    printf("Assertion failed: %s, function %s, file %s, line %d\n",
           msg, fn, file, line);
    abort();
}
