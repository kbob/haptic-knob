#ifndef STEVAL_IHM043V1_included
#define STEVAL_IHM043V1_included

// STMicroelectronics STEVAL-IHM043V1
// 6-step BLDC sensorless driver board based on the STM32F051 and L624

#include <stddef.h>

#include "gpio.h"
#include "timer.h"

#ifndef STM32F0
    # error "STEVAL_IHM043V1 requires STM32F0 definitions"
#endif

// LED
extern const gpio_pin           steval_ihm043v1_led_gpios[];
extern const size_t             steval_ihm043v1_led_gpio_count;
#define TARGET_led_gpios        steval_ihm043v1_led_gpios
#define TARGET_led_gpio_count   steval_ihm043v1_led_gpio_count

// USART
extern const gpio_pin           steval_ihm043v1_usart_gpios[];
extern const size_t             steval_ihm043v1_usart_gpio_count;
#define TARGET_usart_gpios      steval_ihm043v1_usart_gpios
#define TARGET_usart_gpio_count steval_ihm043v1_usart_gpio_count

// Timer
extern const gpio_pin           steval_ihm043v1_timer_gpios[];
extern const size_t             steval_ihm043v1_timer_gpio_count;
extern const timer              steval_ihm043v1_timer;
#define TARGET_timer_gpios      steval_ihm043v1_timer_gpios
#define TARGET_timer_gpio_count steval_ihm043v1_timer_gpio_count
#define TARGET_timer            steval_ihm043v1_timer

#endif /* !STEVAL_IHM043C1_included */
