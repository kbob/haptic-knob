#include "steval-ihm043v1.h"

const gpio_pin TARGET_led_gpios[] = {
    {
        .gp_port = GPIOA,
        .gp_pin  = GPIO11,
        .gp_mode = GPIO_MODE_OUTPUT,
    },
};

const size_t TARGET_led_gpio_count = (&TARGET_led_gpios)[1] - TARGET_led_gpios;
