#include "gpio.h"

#include <assert.h>
#include <stdbool.h>

#include <libopencm3/stm32/rcc.h>

#define GPIO_PORT_COUNT 11

static uint16_t gpio_pins_used[GPIO_PORT_COUNT];

static uint32_t gpio_clocks[] = {
    RCC_GPIOA,
    RCC_GPIOB,
    RCC_GPIOC,
    RCC_GPIOD,
    RCC_GPIOE,
    RCC_GPIOE,
    RCC_GPIOF,
#ifdef RCC_GPIOH
    RCC_GPIOH,
    RCC_GPIOI,
    RCC_GPIOJ,
    RCC_GPIOK,
#endif
};
static const size_t gpio_clock_count = (&gpio_clocks)[1] - gpio_clocks;

void gpio_init_pin(const gpio_pin *pin)
{
    uint32_t port = pin->gp_port;
    uint16_t pinmask = pin->gp_pin;
    uint32_t index = ((uint32_t)port - (uint32_t)GPIOA) >> 10;
    assert(index < GPIO_PORT_COUNT);

    if (!gpio_pins_used[index]) {
        assert(index < gpio_clock_count);
        rcc_periph_clock_enable(gpio_clocks[index]);
    }

    assert(!(gpio_pins_used[index] & pinmask));
    gpio_pins_used[index] |= pinmask;

    gpio_mode_setup(port,
                    pin->gp_mode,
                    pin->gp_pupd,
                    pinmask);

    if (pin->gp_mode == GPIO_MODE_OUTPUT) {
        if (pin->gp_level)
            gpio_set(port, pinmask);
        else
            gpio_clear(port, pinmask);
    }

    if (pin->gp_mode == GPIO_MODE_OUTPUT || pin->gp_mode == GPIO_MODE_AF)
        gpio_set_output_options(port,
                                pin->gp_otype,
                                pin->gp_ospeed,
                                pinmask);

    if (pin->gp_mode == GPIO_MODE_AF)
        gpio_set_af(port,
                    pin->gp_af,
                    pinmask);
}

void gpio_init_pins(const gpio_pin *pins, size_t count)
{
    for (size_t i = 0; i < count; i++)
        gpio_init_pin(&pins[i]);
}
