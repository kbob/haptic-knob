#ifndef STEVAL_IHM043V1_included
#define STEVAL_IHM043V1_included

// STMicroelectronics STEVAL-IHM043V1
// 6-step BLDC sensorless driver board based on the STM32F051 and L624

#include <stddef.h>

#include <libopencm3/stm32/exti.h>

#include "gpio.h"
#include "timer.h"
#include "usart.h"

#ifndef STM32F0
    # error "STEVAL_IHM043V1 requires STM32F0 definitions"
#endif

// LED
extern const gpio_pin           steval_ihm043v1_led_gpios[];
extern const size_t             steval_ihm043v1_led_gpio_count;
#define TARGET_led_gpios        steval_ihm043v1_led_gpios
#define TARGET_led_gpio_count   steval_ihm043v1_led_gpio_count

// USART
extern const USART_periph       steval_ihm043v1_USART;
#define TARGET_USART            steval_ihm043v1_USART

// Timer
extern const timer_periph       steval_ihm043v1_advanced_timer;
#define TARGET_advanced_timer   steval_ihm043v1_advanced_timer

// Software Interrupt
#define STEVAL_IHM043V1_SW_EXTI EXTI2
extern void                     steval_ihm043v1_trigger_sw_interrupt(void);
#define TARGET_SW_EXTI          STEVAL_IHM043V1_SW_EXTI
#define TARGET_SW_IRQ           EXTI2_3_IRQ
#define TARGET_sw_isr           exti2_3_isr
#define TARGET_trigger_sw_interrupt steval_ihm043v1_trigger_sw_interrupt

#endif /* !STEVAL_IHM043C1_included */
