#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include "led.h"
#include "systick.h"

static uint32_t next_time;

static void handle_systick(uint32_t millis)
{
    if (millis >= next_time) {
        LED_toggle();
        next_time += 500;
    }
}

int main(void)
{
    rcc_clock_setup_in_hsi_out_48mhz();
    init_systick(rcc_ahb_frequency);
    init_LED();

    next_time = system_millis;
    register_systick_handler(handle_systick);

    while (1)
        continue;
}
