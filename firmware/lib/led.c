#include "led.h"

#include "gpio.h"
#include TARGET_H

void init_LED(void)
{
    gpio_init_pins(TARGET_led_gpios, TARGET_led_gpio_count);
}

void LED_on(void)
{
    const gpio_pin *gpios = &TARGET_led_gpios[0];
    gpio_clear(gpios->gp_port, gpios->gp_pin);
}

void LED_off(void)
{
    const gpio_pin *gpios = &TARGET_led_gpios[0];
    gpio_set(gpios->gp_port, gpios->gp_pin);
}

void LED_set(bool on_off)
{
    if (on_off)
        LED_on();
    else
        LED_off();
}

void LED_toggle(void)
{
    const gpio_pin *gpios = &TARGET_led_gpios[0];
    gpio_toggle(gpios->gp_port, gpios->gp_pin);
}
