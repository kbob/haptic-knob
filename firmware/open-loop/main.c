#include <assert.h>
#include <stdlib.h>

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>

#include "math.h"
#include "printf.h"
#include "systick.h"
#include "usart.h"
#include TARGET_H

// const int USART_BAUD = 115200;

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

static uint8_t phase;
static uint32_t muphase;

static void commutate(void)
{
    switch (phase) {

    case 0:                     // phase 0 - A, B positive
        gpio_set(GPIOA, GPIO8 | GPIO10);
        gpio_clear(GPIOA, GPIO9);
        break;

    case 1:
        gpio_set(GPIOA, GPIO8);
        gpio_clear(GPIOA, GPIO9 | GPIO10);
        break;

    case 2:
        gpio_set(GPIOA, GPIO8 | GPIO9);
        gpio_clear(GPIOA, GPIO10);
        break;

    case 3:
        gpio_set(GPIOA, GPIO9);
        gpio_clear(GPIOA, GPIO8 | GPIO10);
        break;

    case 4:
        gpio_set(GPIOA, GPIO9 | GPIO10);
        gpio_clear(GPIOA, GPIO8);
        break;

    case 5:
        gpio_set(GPIOA, GPIO10);
        gpio_clear(GPIOA, GPIO8 | GPIO9);
        break;

    default:
        assert(0 && "invalid phase");
    }
}

extern void tim1_brk_up_trg_com_isr(void)
{
    timer_clear_flag(TIM1, TIM_SR_UIF);

    // phase: 0..6
    // angle: 0..1024
    // muphase: 0..(240 * 1024)
    // phase = muphase / (40 * 1024)
    // angle = muphase / 1024
    // phase to 6 in 240 steps
    // muphase to (240 * 1024) in 240 steps; step size 1024
    // angle to 1024 in 240 steps; step size 1024 / 240
    // phase to 6 in 240 steps; step size 1 / 40
    muphase += 1024;
    if (muphase >= (240 * 1024))
        muphase -= (240 * 1024);
    uint8_t nphase = muphase / (40 * 1024);

    if (phase != nphase) {
        phase = nphase;
        commutate();
    }

    // XXX calc once
    // XXX add amplitude parameter
    uint32_t period = timer_period(&motor_timer);

    uint32_t angle_a = (muphase - 0 * 1024) / 240;
    uint16_t duty_a = (abs(sini16(angle_a)) * (period - 2)) >> 15;
    timer_set_pwm_duty(&motor_timer, TIM_OC1, duty_a);
    uint32_t angle_b = (muphase + 160 * 1024) / 240;
    uint16_t duty_b = (abs(sini16(angle_b)) * (period - 2)) >> 15;
    timer_set_pwm_duty(&motor_timer, TIM_OC2, duty_b);
    uint32_t angle_c = (muphase + 80 * 1024) / 240;
    uint16_t duty_c = (abs(sini16(angle_c)) * (period - 2)) >> 15;
    timer_set_pwm_duty(&motor_timer, TIM_OC3, duty_c);
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
    init_timer(&motor_timer);
    timer_enable_pwm(&motor_timer, TIM_OC1);
    timer_enable_pwm(&motor_timer, TIM_OC2);
    timer_enable_pwm(&motor_timer, TIM_OC3);
    timer_set_pwm_duty(&motor_timer, TIM_OC1, timer_period(&motor_timer) / 2);
    timer_set_pwm_duty(&motor_timer, TIM_OC2, timer_period(&motor_timer) / 2);
    timer_set_pwm_duty(&motor_timer, TIM_OC3, timer_period(&motor_timer) / 2);
    timer_enable_irq(motor_timer.periph->base, TIM_DIER_UIE);
    nvic_enable_irq(NVIC_TIM1_BRK_UP_TRG_COM_IRQ);

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
    printf("Assertion failed: %s, function %s, file %s, line %d\n",
           msg, fn, file, line);
    abort();
}
