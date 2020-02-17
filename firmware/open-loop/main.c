#include <assert.h>
#include <stdlib.h>

#include <libopencm3/stm32/rcc.h>

#include "dda.h"
#include "math.h"
#include "printf.h"
#include "regdump.h"
#include "systick.h"
#include "usart.h"
#include TARGET_H

#undef OLD_MOTOR

static const USART console_USART = {
    .periph           = &TARGET_USART,
    .baud             = 115200,
};

static L6234_config motor_config = {
    .pwm_frequency    = 40000,
    .speed_resolution = 80000,
};

static void handle_systick(uint32_t millis)
{
    millis = millis;            // -Wunused-parameter
}

static DDA_state motor_position_DDA;

extern void TARGET_advanced_timer_up_isr(void)
{
    uint32_t tim = TARGET_L6234.timer->base;
    timer_clear_flag(tim, TIM_SR_UIF);
    assert(!timer_get_flag(tim, TIM_SR_UIF));

    L6234_handle_timer_interrupt(&TARGET_L6234);

    assert(!timer_get_flag(tim, TIM_SR_UIF));
}

extern void TARGET_sw_isr(void)
{
    exti_reset_request(TARGET_SWINT_EXTI);

    L6234_handle_sw_interrupt(&TARGET_L6234);

    int32_t pos = DDA_next(&motor_position_DDA);
    if (pos >= L6234_CIRCLE)
        motor_position_DDA.y -= L6234_CIRCLE;
    L6234_set_position(&TARGET_L6234, pos);

    // assert(!timer_get_flag(TARGET_L6234.timer->base, TIM_SR_UIF));
}

int main(void)
{
    rcc_clock_setup_in_hse_8mhz_out_48mhz();
    init_USART(&console_USART);
    printf("\n\n\n\n\n");
    printf("rcc_ahb_frequency  = %lu\n", rcc_ahb_frequency);
    printf("rcc_apb1_frequency = %lu\n", rcc_apb1_frequency);

    init_sin_table();
    init_DDA(&motor_position_DDA, 40000, L6234_CIRCLE, 0);

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

    init_L6234(&TARGET_L6234, &motor_config);
    L6234_set_amplitude(&TARGET_L6234, L6234_AMPLITUDE_MAX / 1);
    uint32_t next_time = system_millis;

    while (1) {

        if (system_millis >= next_time) {
            next_time += 100;
            rewind_advanced_timer_registers();
            dump_advanced_timer_registers(TARGET_L6234.timer->base);
        }
    }
}
