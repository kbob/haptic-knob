#include <assert.h>
#include <stdlib.h>

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>

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

static const uint32_t phase_polarity[6] = {
        GPIO8 | GPIO9,
        GPIO8,
        GPIO8         | GPIO10,
                        GPIO10,
                GPIO9 | GPIO10,
                GPIO9,
};

volatile uint32_t cc_counter;
volatile uint32_t sw_counter;

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

extern void tim1_cc_isr(void)
{
    timer_clear_flag(TIM1, TIM_SR_UIF | TIM_SR_CC4IF);
    cc_counter++;
    assert(!timer_get_flag(TIM1, TIM_SR_UIF));

    TARGET_trigger_sw_interrupt();

    if (pwm_update.pending) {
        timer_set_pwm_duty(&motor_timer, TIM_OC1, pwm_update.width_a);
        timer_set_pwm_duty(&motor_timer, TIM_OC2, pwm_update.width_b);
        timer_set_pwm_duty(&motor_timer, TIM_OC3, pwm_update.width_c);
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

__attribute__((optimize("O3")))
extern void TARGET_sw_isr(void)
{
    exti_reset_request(TARGET_SW_EXTI);
    sw_counter++;
    if (pwm_update.pending)
        return;

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
    assert(!timer_get_flag(TIM1, TIM_SR_UIF));
}

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
