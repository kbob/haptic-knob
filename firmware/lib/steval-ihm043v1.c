#include "steval-ihm043v1.h"

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>

#define ARRAY_COUNT(a) ((&(a))[1] - (a))

const gpio_pin TARGET_led_gpios[] = {
    {
        .gp_port = GPIOA,
        .gp_pin  = GPIO11,
        .gp_mode = GPIO_MODE_OUTPUT,
    },
};
const size_t TARGET_led_gpio_count = ARRAY_COUNT(TARGET_led_gpios);

const USART_periph TARGET_USART = {
    .base = USART1,
    .clock = RCC_USART1,
    .tx = {                     // PB6 AF0 USART1_TX
        .gp_port = GPIOB,
        .gp_pin  = GPIO6,
        .gp_mode = GPIO_MODE_AF,
        .gp_af   = GPIO_AF0,

    },
    .rx = {                     // PB7 AF0 USART1_RX
        .gp_port = GPIOB,
        .gp_pin  = GPIO7,
        .gp_mode = GPIO_MODE_AF,
        .gp_af   = GPIO_AF0,
    },
};

static timer_oc tim1_out_channels[] = {
    [TIM_OC1] = {               // PA8 AF2 TIM1_CH1 -> L6234 INA
        .id          = TIM_OC1,
        .is_inverted = false,
        .gpio = {
            .gp_port = GPIOA,
            .gp_pin  = GPIO8,
            .gp_mode = GPIO_MODE_AF,
            .gp_af   = GPIO_AF2,
        },
    },
    [TIM_OC1N] = {              // PB13 AF2 TIM1_CH1N -> L6234 ENA
        .id          = TIM_OC1N,
        .is_inverted = false,
        .gpio = {
            .gp_port = GPIOB,
            .gp_pin  = GPIO13,
            .gp_mode = GPIO_MODE_AF,
            .gp_af   = GPIO_AF2,
        },
    },
    [TIM_OC2] = {               // PA9 AF2 TIM1_CH2 -> L6234 INB
        .id          = TIM_OC2,
        .is_inverted = false,
        .gpio = {
            .gp_port = GPIOA,
            .gp_pin  = GPIO9,
            .gp_mode = GPIO_MODE_AF,
            .gp_af   = GPIO_AF2,
        },
    },
    [TIM_OC2N] = {              // PB14 AF2 TIM1_CH2N -> L6234 ENB
        .id          = TIM_OC2N,
        .is_inverted = false,
        .gpio = {
            .gp_port = GPIOB,
            .gp_pin  = GPIO14,
            .gp_mode = GPIO_MODE_AF,
            .gp_af   = GPIO_AF2,
        },
    },
    [TIM_OC3] = {               // PA10 AF2 TIM1_CH3 -> L6234 INC
        .id          = TIM_OC3,
        .is_inverted = false,
        .gpio = {
            .gp_port = GPIOA,
            .gp_pin  = GPIO10,
            .gp_mode = GPIO_MODE_AF,
            .gp_af   = GPIO_AF2,
        },
    },
    [TIM_OC3N] = {              // PB15 AF2N TIM1_CH3N -> L6234 ENC
        .id          = TIM_OC3N,
        .is_inverted = false,
        .gpio = {
            .gp_port = GPIOB,
            .gp_pin  = GPIO15,
            .gp_mode = GPIO_MODE_AF,
            .gp_af   = GPIO_AF2,
        },
    },
    [TIM_OC4] = {               // PA11 AF2 TIM1_CH4 -> LED (active low)
        .id          = TIM_OC4,
        .is_inverted = true,
        .gpio = {
            .gp_port = GPIOA,
            .gp_pin  = GPIO11,
            .gp_mode = GPIO_MODE_AF,
            .gp_af   = GPIO_AF2,
        },
    },
};

timer_periph TARGET_advanced_timer = {
    .base       = TIM1,
    .clock      = RCC_TIM1,
    .out_channels = tim1_out_channels,
    .out_channel_count = ARRAY_COUNT(tim1_out_channels),
};

L6234_periph TARGET_L6234 = {
    .timer               = &TARGET_advanced_timer,
    .channels = {
        [0] = {
            .ena_id      = TIM_OC1N,
            .dir_pin     = {
                .gp_port = GPIOA,
                .gp_pin  = GPIO8,
                .gp_mode = GPIO_MODE_OUTPUT,
            },
        },
        [1] = {
            .ena_id      = TIM_OC2N,
            .dir_pin     = {
                .gp_port = GPIOA,
                .gp_pin  = GPIO9,
                .gp_mode = GPIO_MODE_OUTPUT,
            },
        },
        [2] = {
            .ena_id      = TIM_OC3N,
            .dir_pin     = {
                .gp_port = GPIOA,
                .gp_pin  = GPIO10,
                .gp_mode = GPIO_MODE_OUTPUT,
            },
        },
    },
};

void TARGET_trigger_sw_interrupt(void)
{
    EXTI_SWIER = TARGET_SWINT_EXTI;
}
