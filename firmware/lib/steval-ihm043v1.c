#include "steval-ihm043v1.h"

#include <libopencm3/stm32/rcc.h>

#define ARRAY_COUNT(a) ((&(a))[1] - (a))

const gpio_pin TARGET_led_gpios[] = {
    {
        .gp_port = GPIOA,
        .gp_pin  = GPIO11,
        .gp_mode = GPIO_MODE_OUTPUT,
    },
};
const size_t TARGET_led_gpio_count = ARRAY_COUNT(TARGET_led_gpios);

const gpio_pin TARGET_usart_gpios[] = {
    {                           // PB6 TX
        .gp_port = GPIOB,
        .gp_pin  = GPIO6,
        .gp_mode = GPIO_MODE_AF,
        .gp_af   = GPIO_AF0,
    },
    {                           // PB7 RX
        .gp_port = GPIOB,
        .gp_pin  = GPIO7,
        .gp_mode = GPIO_MODE_AF,
        .gp_af   = GPIO_AF0,
    },
};
const size_t TARGET_usart_gpio_count = ARRAY_COUNT(TARGET_usart_gpios);

static const timer_oc tim1_out_channels[] = {
    [TIM_OC1] = {               // OC1 - PA8 -> L6234 INA
        .id          = TIM_OC1,
        .is_inverted = false,
        .gpio = {
            .gp_port = GPIOA,
            .gp_pin  = GPIO8,
            .gp_mode = GPIO_MODE_AF,
            .gp_af   = GPIO_AF2,
        },
    },
    [TIM_OC1N] = {              // OC1N - PB13 -> L6234 ENA
        .id          = TIM_OC1N,
        .is_inverted = false,
        .gpio = {
            .gp_port = GPIOB,
            .gp_pin  = GPIO13,
            .gp_mode = GPIO_MODE_AF,
            .gp_af   = GPIO_AF2,
        },
    },
    [TIM_OC2] = {               // OC2 - PA9 -> L6234 INB
        .id          = TIM_OC2,
        .is_inverted = false,
        .gpio = {
            .gp_port = GPIOA,
            .gp_pin  = GPIO9,
            .gp_mode = GPIO_MODE_AF,
            .gp_af   = GPIO_AF2,
        },
    },
    [TIM_OC2N] = {              // OC1N - PB14 -> L6234 ENB
        .id          = TIM_OC2N,
        .is_inverted = false,
        .gpio = {
            .gp_port = GPIOB,
            .gp_pin  = GPIO14,
            .gp_mode = GPIO_MODE_AF,
            .gp_af   = GPIO_AF2,
        },
    },
    [TIM_OC3] = {               // OC3 - PA10 -> L6234 INC
        .id          = TIM_OC3,
        .is_inverted = false,
        .gpio = {
            .gp_port = GPIOA,
            .gp_pin  = GPIO10,
            .gp_mode = GPIO_MODE_AF,
            .gp_af   = GPIO_AF2,
        },
    },
    [TIM_OC3N] = {              // OC3N - PB15 -> L6234 ENC
        .id          = TIM_OC3N,
        .is_inverted = false,
        .gpio = {
            .gp_port = GPIOB,
            .gp_pin  = GPIO15,
            .gp_mode = GPIO_MODE_AF,
            .gp_af   = GPIO_AF2,
        },
    },
    [TIM_OC4] = {
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

const timer_periph TARGET_advanced_timer = {
    .base       = TIM1,
    .clock      = RCC_TIM1,
    .out_channels = tim1_out_channels,
    .out_channel_count = ARRAY_COUNT(tim1_out_channels),
};
