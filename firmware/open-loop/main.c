#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>     // XXX

#include "math.h"
#include "printf.h"
#include "systick.h"
#include "timer.h"
#include "usart.h"
#include TARGET_H

// #define MOTOR_OUT_TEST

const int USART_BAUD = 115200;

// const timer_config timer_cfg = {
//     .period       = 48000000 / 4000,
//     .enable_LED   = true,
//     .enable_motor = false,
// };

#ifdef MOTOR_OUT_TEST

typedef enum motor_gpio {
    INA, INB, INC,
    ENA, ENB, ENC,
} motor_gpio;

static const gpio_pin motor_gpios[] = {
    {
        .gp_port = GPIOA,
        .gp_pin  = GPIO8 | GPIO9 | GPIO10,
        .gp_mode = GPIO_MODE_OUTPUT,
    },
    {
        .gp_port = GPIOB,
        .gp_pin = GPIO13 | GPIO14 | GPIO15,
        .gp_mode = GPIO_MODE_OUTPUT,
    },
};
static const int motor_gpio_count = (&motor_gpios)[1] - motor_gpios;

#else

static const timer motor_timer = {
    .periph         = &TARGET_advanced_timer,
    .pwm_freq       = 40000,
    .enable_outputs = TOB_OC1N | TOB_OC2N | TOB_OC3N,
    // .enable_outputs = (
    //                       TOB_OC1 | TOB_OC1N |
    //                       TOB_OC2 | TOB_OC2N |
    //                       TOB_OC3 | TOB_OC3N
    //                   ),
};

#endif /* MOTOR_OUT_TEST */

static void handle_systick(uint32_t millis)
{
    static uint32_t next_time;
    if (millis >= next_time) {
        // if (timer_cfg.enable_LED) {
        //     uint32_t idx = (millis % 1000 * CIRCLE_SUBDIVISION) / 1000;
        //     int32_t s = sini16(idx);
        //     s = s > 0 ? s : 0;                  // rectify
        //     uint32_t v = (s * s) >> 14;         // gamma correct
        //     v *= timer_cfg.period; v >>= 16;    // scale
        //     timer_set_LED_duty(v);
        // }
        next_time += 1;
    }
}

#ifdef MOTOR_OUT_TEST

static void set(motor_gpio g, bool val)
{
    static const uint32_t ports[6] = {
        [INA] = GPIOA,
        [INB] = GPIOA,
        [INC] = GPIOA,
        [ENA] = GPIOB,
        [ENB] = GPIOB,
        [ENC] = GPIOB,
    };
    static const uint32_t pins[6] = {
        [INA] = GPIO8,
        [INB] = GPIO9,
        [INC] = GPIO10,
        [ENA] = GPIO13,
        [ENB] = GPIO14,
        [ENC] = GPIO15,
    };
    (val ? gpio_set : gpio_clear)(ports[g], pins[g]);
}

#else

// static void set(enum tim_oc_id oc, bool val)
// {
//     (val ? timer_force_output_high : timer_force_output_low)(&motor_timer, oc);
// }

// static void set(enum tim_oc_id oc, bool val)
// {
//     const timer_periph *tpp = motor_timer.periph;
//     assert(oc < tpp->out_channel_count);
//     const timer_oc *tocp = &tpp->out_channels[oc];
//     const gpio_pin *tocgp = &tocp->gpio;
//     (val ? gpio_set : gpio_clear)(tocgp->gp_port, tocgp->gp_pin);
// }

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

#endif /* MOTOR_OUT_TEST */

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
    uint32_t period = timer_period(&motor_timer);
    // uint32_t inv_period = 65536 / period;

    uint32_t angle_a = (muphase - 0 * 1024) / 240;
    uint16_t duty_a = (abs(sini16(angle_a)) * (period - 2)) >> 15;
    timer_set_pwm_duty(&motor_timer, TIM_OC1, duty_a);
    uint32_t angle_b = (muphase + 160 * 1024) / 240;
    uint16_t duty_b = (abs(sini16(angle_b)) * (period - 2)) >> 15;
    timer_set_pwm_duty(&motor_timer, TIM_OC2, duty_b);
    uint32_t angle_c = (muphase + 80 * 1024) / 240;
    uint16_t duty_c = (abs(sini16(angle_c)) * (period - 2)) >> 15;
    timer_set_pwm_duty(&motor_timer, TIM_OC3, duty_c);

    // static uint32_t prev_ms;
    // uint32_t ms = system_millis;
    // if (prev_ms == ms)
    //     return;
    // prev_ms = ms;
    // phase = (phase + 1) % 6;
    // commutate();
}

int main(void)
{
    rcc_clock_setup_in_hse_8mhz_out_48mhz();
    init_USART(USART_BAUD);
    printf("\n\n\n\n\n");
    printf("rcc_ahb_frequency = %lu\n", rcc_ahb_frequency);
    printf("rcc_apb1_frequency = %lu\n", rcc_apb1_frequency);

    init_sin_table();
    init_systick(rcc_ahb_frequency);
    register_systick_handler(handle_systick);
#ifdef MOTOR_OUT_TEST
    gpio_init_pins(motor_gpios, motor_gpio_count);
#else
    init_timer(&motor_timer);
    timer_enable_pwm(&motor_timer, TIM_OC1);
    timer_enable_pwm(&motor_timer, TIM_OC2);
    timer_enable_pwm(&motor_timer, TIM_OC3);
    timer_set_pwm_duty(&motor_timer, TIM_OC1, timer_period(&motor_timer) / 2);
    timer_set_pwm_duty(&motor_timer, TIM_OC2, timer_period(&motor_timer) / 2);
    timer_set_pwm_duty(&motor_timer, TIM_OC3, timer_period(&motor_timer) / 2);
    timer_enable_irq(motor_timer.periph->base, TIM_DIER_UIE);
    nvic_enable_irq(NVIC_TIM1_BRK_UP_TRG_COM_IRQ);
    // timer_force_output_high(&motor_timer, TIM_OC2);
    // timer_force_output_low(&motor_timer, TIM_OC3);

    control_gpio(TIM_OC1);
    control_gpio(TIM_OC2);
    control_gpio(TIM_OC3);

#endif

    uint32_t next_time = system_millis;
    uint32_t tick = system_millis;
    while (1) {
        // gpio_toggle(GPIOB, GPIO13);

        if (system_millis > tick) {
            tick++;
#ifdef MOTOR_OUT_TEST
            set(ENA, (bool)(tick & 1));
            set(INA, (bool)(tick & 2));
            set(ENB, (bool)(tick-1 & 1));
            set(INB, (bool)(tick-1 & 2));
            set(ENC, (bool)(tick-2 & 1));
            set(INC, (bool)(tick-2 & 2));
#else
    // phase = (phase + 1) % 6;
    // commutate();
#endif /* MOTOR_OUT_TEST */
        }

        if (system_millis < next_time)
            continue;
        next_time += 1000;

#ifndef MOTOR_OUT_TEST
printf("CR1   %4lx  SR    %4lx  CCER  %4lx  RCR   %4lx  CCR4  %4lx\n",
   TIM1_CR1,   TIM1_SR,    TIM1_CCER,  TIM1_RCR,   TIM1_CCR4);
printf("CR2   %4lx  EGR   %4lx  CNT   %4lx  CCR1  %4lx  BDTR  %4lx\n",
   TIM1_CR2,   TIM1_EGR,   TIM1_CNT,   TIM1_CCR1,  TIM1_BDTR);
printf("SMCR  %4lx  CCMR1 %4lx  PSC   %4lx  CCR2  %4lx  DCR   %4lx\n",
   TIM1_SMCR,  TIM1_CCMR1, TIM1_PSC,   TIM1_CCR2,  TIM1_DCR);
printf("DIER  %4lx  CCMR2 %4lx  ARR   %4lx  CCR3  %4lx  DMAR  %4lx\n",
   TIM1_DIER,  TIM1_CCMR2, TIM1_ARR,   TIM1_CCR3,  TIM1_DMAR);
printf("\n");
printf("\33[A\33[A\33[A\33[A\33[A");
#endif /* !MOTOR_OUT_TEST */
    }
}

void __assert_func(const char *file, int line, const char *fn, const char *msg)
{
    printf("Assertion failed: %s, function %s, file %s, line %d\n",
           msg, fn, file, line);
    abort();
}
