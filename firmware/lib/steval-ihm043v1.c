#include "steval-ihm043v1.h"

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>

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

const gpio_pin TARGET_timer_gpios[] = {
    {                           // PA11 - LED
        .gp_port = GPIOA,
        .gp_pin  = GPIO11,
        .gp_mode = GPIO_MODE_AF,
        .gp_af   = GPIO_AF2,
    },
};
const size_t TARGET_timer_gpio_count = ARRAY_COUNT(TARGET_timer_gpios);

const timer TARGET_timer = {
        .base       = TIM1,
        .clock      = RCC_TIM1,
        .gpios      = TARGET_timer_gpios,
        .gpio_count = ARRAY_COUNT(TARGET_timer_gpios),
};
