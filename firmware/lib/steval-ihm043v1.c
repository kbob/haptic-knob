#include "steval-ihm043v1.h"

const gpio_pin TARGET_led_gpios[] = {
    {
        .gp_port = GPIOA,
        .gp_pin  = GPIO11,
        .gp_mode = GPIO_MODE_OUTPUT,
    },
};
const size_t TARGET_led_gpio_count =
    (&TARGET_led_gpios)[1] - TARGET_led_gpios;

const gpio_pin TARGET_usart_gpios[] = {
    {                           // PB6 TX
        .gp_port = GPIOB,
        .gp_pin  = GPIO6,
        .gp_mode = GPIO_MODE_AF,
        .gp_pupd = GPIO_PUPD_NONE,
        .gp_af   = GPIO_AF0,
    },
    {                           // PB7 RX
        .gp_port = GPIOB,
        .gp_pin  = GPIO7,
        .gp_mode = GPIO_MODE_AF,
        .gp_pupd = GPIO_PUPD_NONE,
        .gp_af   = GPIO_AF0,
    },
};
const size_t TARGET_usart_gpio_count =
    (&TARGET_usart_gpios)[1] - TARGET_usart_gpios;
