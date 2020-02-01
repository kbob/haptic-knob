#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include "systick.h"
#include "steval-ihm043v1.h"

static uint32_t next_time;

static void handle_systick(uint32_t millis)
{
    if (millis >= next_time) {
        gpio_toggle(GPIOA, GPIO11);
        next_time += 500;
    }
}

int main(void)
{
    rcc_clock_setup_in_hsi_out_48mhz();
    init_systick(rcc_ahb_frequency);
    // rcc_periph_clock_enable(RCC_GPIOA);
    // gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO11);
    gpio_init_pins(TARGET_led_gpios, TARGET_led_gpio_count);

    next_time = system_millis;
    register_systick_handler(handle_systick);

    while (1) continue;

    // uint32_t next_time = system_millis;
    // while (1) {
    //     if (system_millis >= next_time) {
    //         gpio_toggle(GPIOA, GPIO11);
    //         next_time += 500;
    //     }
    // }
}
