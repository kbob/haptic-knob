#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>     // XXX

#include "math.h"
#include "printf.h"
#include "systick.h"
#include "timer.h"
#include "usart.h"
#include TARGET_H

const int USART_BAUD = 115200;

const timer_config timer_cfg = {
    .period       = 48000000 / 40000,
    .enable_LED   = true,
    .enable_motor = false,
};

static void handle_systick(uint32_t millis)
{
    static uint32_t next_time;
    if (millis >= next_time) {
        if (timer_cfg.enable_LED) {
            uint32_t idx = (millis % 1000 * CIRCLE_SUBDIVISION) / 1000;
            int32_t s = sini16(idx);
            s = s > 0 ? s : 0;                  // rectify
            uint32_t v = (s * s) >> 14;         // gamma correct
            v *= timer_cfg.period; v >>= 16;    // scale
            timer_set_LED_duty(v);
        }
        next_time += 1;
    }
}

int main(void)
{
    rcc_clock_setup_in_hsi_out_48mhz();
    init_USART(USART_BAUD);
    printf("rcc_ahb_frequency = %lu\n", rcc_ahb_frequency);
    printf("rcc_apb1_frequency = %lu\n", rcc_apb1_frequency);

    init_sin_table();
    init_systick(rcc_ahb_frequency);
    register_systick_handler(handle_systick);
    init_timer(&timer_cfg);

    while (1) {

        printf("CR1   %4lx  SR    %4lx  CCER  %4lx  RCR   %4lx  CCR4  %4lx\n",
           TIM1_CR1,   TIM1_SR,    TIM1_CCER,  TIM1_RCR,   TIM1_CCR4);
        printf("CR2   %4lx  EGR   %4lx  CNT   %4lx  CCR1  %4lx  BDTR  %4lx\n",
           TIM1_CR2,   TIM1_EGR,   TIM1_CNT,   TIM1_CCR1,  TIM1_BDTR);
        printf("SMCR  %4lx  CCMR1 %4lx  PSC   %4lx  CCR2  %4lx  DCR   %4lx\n",
           TIM1_SMCR,  TIM1_CCMR1, TIM1_PSC,   TIM1_CCR2,  TIM1_DCR);
        printf("DIER  %4lx  CCMR2 %4lx  ARR   %4lx  CCR3  %4lx  DMAR  %4lx\n",
           TIM1_DIER,  TIM1_CCMR2, TIM1_ARR,   TIM1_CCR3,  TIM1_DMAR);
        printf("\n");

        for (int i = 0; i < 20000000; i++)
            __asm__ ( "nop" );
    }
}

void __assert_func(const char *file, int line, const char *fn, const char *msg)
{
    printf("Assertion failed: %s, function %s, file %s, line %d\n",
           msg, fn, file, line);
    abort();
}
