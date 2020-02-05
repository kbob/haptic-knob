#include "timer.h"

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>

#include "gpio.h"
#include TARGET_H

// GPIOs
//   PA8  AF2   TIM1_CH1    -> L6234 INA
//   PA9  AF2   TIM1_CH2    -> L6234 INB
//   PA10 AF2   TIM1_CH3    -> L6234 INC
//   PA12 AF2   TIM1_ETR    <- N/C
//
//   PB12 AF2   TIM1_BKIN   -> P6 pin 4
//   PB13 AF2   TIM1_CH1N   -> L6234 ENA
//   PB14 AF2   TIM1_CH2N   -> L6234 ENB
//   PB15 AF2   TIM1_CH3N   -> L6345 ENC

// setup
//
// // Slave Mode Control Register
// //    bit 15 ETP:  External trigger inverted
// //    bits 6:4 TS: trigger selection: external trigger input
// TIM1->SMCR = b15+b4+b5+b6; // make ETR input active low
//
// // Control Register 2
// TIM1->CR2= 0;
//
// // Capture/COmpare Register x
// //    bits 15:0 CCRx: capture/compare x value
// TIM1->CCR1= 200;
// TIM1->CCR2= 200;
// TIM1->CCR3= 200;
// TIM1->CCR4= 1100;
//
// // Auto-Reload Register
// //    bits 15:0 ARR: auto-reload value
// TIM1->ARR=1200;
//
// // Control Register 1
// //    bits 6:5 CMS: edge-aligned mode
// //    bit 4 DIR: count up
// //    bit 0 CEN: counter enabled
// TIM1->CR1=0x0001;
//
// // Capture/Compare Mode Register 1
// // note: b15 b7 and b7 are to enable ETR  based current limit
// //    bit 15 OC2CE:    output compare 2 clear enable
// //    bits 14:12 OC2M: PWM mode 1
// //    bit 11 OC1PE:    output compare 2 preload enable
// //    bit 7 OC1CE:     output compare 1 clear enable
// //    bits 6:4 OC1M:   PWM mode 1
// //    bit 3 OC1PE:     output compare 1 preload enable
// TIM1->CCMR1= 0x6868 +b15 + b7;
//
// // Capture/Compare Mode Register 2
// // note: b15 b7 and b7 are to enable ETR  based current limit
// //    bits 14:12 OC4M: PWM mode 1
// //    bit 11 OC3PE:    output compare 4 preload enable
// //    bit 7 OC3CE:     output compare 3 clear enable
// //    bits 6:4 OC3M:   PWM mode 1
// //    bit 3 OC3PE:     output compare 3 preload enable
// TIM1->CCMR2= 0x6868 +b7;
//
// // DMA/Interrupt Enable Register
// //    bit 6 TIE:   trigger interrupt enable
// //    bit 4 CC4IE: capture/compare 4 interrupt enable
// TIM1->DIER = b4+b6;


// // test overcurrent fault condition
// // Break and Dead-Time Register
// //    bit 15 MOE: main output enable
// if ((TIM1->BDTR & b15) == 0)

// // force stop
// // Break and Dead-Time Register
// //    bit 15 MOE: main output enable
// //    bit 13 BP: break polarity active high
// //    bit 12 BKE: break enable
// //    bit 11 OSSR: off-state selection for Run mode
// //                 when inactive, OC/OCN outputs enabled w/ inactive level
// //    bit 10 OSSI: off-state selection for Idle mode
// //                 when inactive, OC/OCH outputs are forced
// TIM1->BDTR= b15+b11+b10+b12+b13;  // set MOE

// // on commutate:
//
// // Capture/Compare Enable Register
// //    bit 11 CC3NP: CC3N output polarity active high on phases 0-3
// //    bit 10 CC3NE: CC3N output enable
// switch(phase)
//     {
//     case 0: // phase AB
//         // enable all 6 except AN
//         // invert AN
//         TIM1->CCER=b10+b8+b6+b4+b0  +b3;
//         TIM1->CCMR1=0x4868+b15+b7; // B low, A PWM
//         TIM1->CCMR2= 0x6858 +b7; // force C ref high (phc en low)
//         break;
//     case 1: // phase AC
//         // enable all 6 except AN
//         // invert AN
//         TIM1->CCER=b10+b8+b6+b4+b0  +b3;
//         TIM1->CCMR1=0x5868+b15+b7; // force B high and A PWM
//         TIM1->CCMR2= 0x6848 +b7; // force C ref low
//         break;
//     case 2: // phase BC
//         // enable all 6 except BN
//         // invert BN
//         TIM1->CCER=b10+b8+b4+b2+b0 +b7;
//         TIM1->CCMR1=0x6858+b15+b7; // force B PWM and A high
//         TIM1->CCMR2= 0x6848 +b7; // force C ref low
//         break;
//     case 3: // phase BA
//         // enable all 6 except BN
//         // invert BN
//         TIM1->CCER=b10+b8+b4+b2+b0 +b7;
//         TIM1->CCMR1=0x6848+b15+b7; // force B PWM and A ref low
//         TIM1->CCMR2= 0x6858 +b7; // force C ref high
//         break;
//     case 4: // phase CA
//         // enable all 6 except CN
//         // invert CN
//         TIM1->CCER=b8+b6+b4+b2+b0 +b11; // enable all 6 except CN
//         TIM1->CCMR1=0x5848+b15+b7; // force B high and A ref low
//         TIM1->CCMR2= 0x6868 +b7; // force C PWM
//         break;
//     case 5: // phase CB
//         // enable all 6 except CN
//         // invert CN
//         TIM1->CCER=b8+b6+b4+b2+b0 +b11; // enable all 6 except CN
//         TIM1->CCMR1=0x4858+b15+b7; // force B low and A high
//         TIM1->CCMR2= 0x6868 +b7; // force C PWM
//         break;
//     } // end of phase switch statement

void init_timer(const timer_config *cfg)
{
    cfg = cfg;
    uint32_t tim = TARGET_timer.base;
    enum tim_oc_id oc = TIM_OC4;
    rcc_periph_clock_enable(TARGET_timer.clock);
    gpio_init_pins(TARGET_timer.gpios, TARGET_timer.gpio_count);

    timer_set_mode(tim, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
    timer_set_prescaler(tim, 0);
    timer_set_repetition_counter(tim, 0);
    timer_disable_preload(tim);
    timer_continuous_mode(tim);
    timer_set_oc_value(tim, oc, 200);      // TIM->CCR4 = 200;
    timer_set_period(tim, 1200);                // TIM->ARR = 1200;
    timer_set_oc_mode(tim, oc, TIM_OCM_PWM1);
    timer_set_oc_polarity_low(tim, oc);
    timer_enable_oc_preload(tim, oc);
    timer_enable_oc_output(tim, oc);
    timer_enable_break_main_output(TIM1);
    timer_enable_counter(tim);                  // TIM->CR1 |= 0b1;
    timer_generate_event(tim, TIM_EGR_UG);
}
