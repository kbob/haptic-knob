#include "timer.h"

#include <assert.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>

#include "gpio.h"

// // These notes decode ST's BLDC sensorless firmware, STSW-IHM043V1.
// // The once-commented lines are from the firmware source; the
// // twice-commented lines are my explanation of what all the register
// // bit assignments are doing.

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
// // SMCR - Slave Mode Control Register
// //    bit 15 ETP:  External trigger inverted
// //    bits 6:4 TS: trigger selection: external trigger input
// TIM1->SMCR = b15+b4+b5+b6; // make ETR input active low
//
// // CR2 - Control Register 2
// TIM1->CR2= 0;
//
// // CCRx - Capture/COmpare Register x
// //    bits 15:0 CCRx: capture/compare x value
// TIM1->CCR1= 200;
// TIM1->CCR2= 200;
// TIM1->CCR3= 200;
// TIM1->CCR4= 1100;
//
// // ARR - Auto-Reload Register
// //    bits 15:0 ARR: auto-reload value
// TIM1->ARR=1200;
//
// // CR1 - Control Register 1
// //    bits 6:5 CMS: edge-aligned mode
// //    bit 4 DIR: count up
// //    bit 0 CEN: counter enabled
// TIM1->CR1=0x0001;
//
// // CCMR1 - Capture/Compare Mode Register 1
// // note: b15 b7 and b7 are to enable ETR  based current limit
// //    bit 15 OC2CE:    output compare 2 clear enable
// //    bits 14:12 OC2M: PWM mode 1
// //    bit 11 OC1PE:    output compare 2 preload enable
// //    bit 7 OC1CE:     output compare 1 clear enable
// //    bits 6:4 OC1M:   PWM mode 1
// //    bit 3 OC1PE:     output compare 1 preload enable
// TIM1->CCMR1= 0x6868 +b15 + b7;
//
// // CCMR2 - Capture/Compare Mode Register 2
// // note: b15 b7 and b7 are to enable ETR  based current limit
// //    bits 14:12 OC4M: PWM mode 1
// //    bit 11 OC3PE:    output compare 4 preload enable
// //    bit 7 OC3CE:     output compare 3 clear enable
// //    bits 6:4 OC3M:   PWM mode 1
// //    bit 3 OC3PE:     output compare 3 preload enable
// TIM1->CCMR2= 0x6868 +b7;
//
// // DIER - DMA/Interrupt Enable Register
// //    bit 6 TIE:   trigger interrupt enable
// //    bit 4 CC4IE: capture/compare 4 interrupt enable
// TIM1->DIER = b4+b6;


// // motorstartinit
//
// CCER - Capture/Compare Enable Register
// //    bits 13:0 disable all
// TIM1->CCER = 0;
//
// // BDTR - Break and Dead-Time Register
// // b12 to enable brk input
// // b13 for break polarity
// // (b15+b11);  //  set MOE
// //    bit 15    MOE:   main output enable
// //    bit 13    BKP:   break polarity active high
// //    bit 12    BKE:   break enable
// //    bit 11    OSSR:  off-state selection for Run mode
// //                     when inactive, OC/OCN enabled w/ inactive level
// //    bit 10    OSSI:  off-state selection for Idle mode
// //                     when inactive, OC/OCH outputs are forced
// TIM1->BDTR= b15+b11+b10+b12+b13;  //  set MOE


// // test overcurrent fault condition
//
// // BDTR - Break and Dead-Time Register
// //    bit 15 MOE:   main output enable
// if ((TIM1->BDTR & b15) == 0)


// // force stop
//
// // Break and Dead-Time Register
// //    bit 15    MOE:   main output enable
// //    bit 13    BKP:   break polarity active high
// //    bit 12    BKE:   break enable
// //    bit 11    OSSR:  off-state selection for Run mode
// //                     when inactive, OC/OCN enabled w/ inactive level
// //    bit 10    OSSI:  off-state selection for Idle mode
// //                     when inactive, OC/OCH outputs are forced
// TIM1->BDTR= b15+b11+b10+b12+b13;  // set MOE


// // commutate2:
//
// // CCER - Capture/Compare Enable Register
// //    bit 11    CC3NP: OC3N output polarity active high on phases 0-3
// //    bit 10    CC3NE: OC3N output enable on phases 0-3
// //    bit  8    CC3E:  OC3 output enable (all phases)
// //    bit  7    CC2NP: OC2N output polarity active high on phases 4-5, 0-1
// //    bit  6    CC2NE: OC2N output enable on phases 4-5, 0-1
// //    bit  4    CC2E:  OC2 output enable (all phases)
// //    bit  3    CC1NP: OC1N output polarity active high on phases 2-5
// //    bit  2    CC1NE: OC1N output enable on phases 2-5
// //    bit  0    CC1E:  OC1 output enable (all phases)
// // CCMR1 - Capture/Compare Mode Register 1
// //    bit 15    OC2CE: OC2Ref cleared immediately on ETRF high
// //    bit 14:12 OC2M:  output compare 2 mode
// //                      100 - force low  (phases 5, 0)
// //                      101 - force high (phases 1, 4)
// //                      110 - PWM mode 1 (phases 2-3)
// //    bit 11    OC2PE: output compare 2 preload enable (all phases)
// //    bit  7    OC1CE: OC1Ref cleared immediately on ETRF high (all phases)
// //    bits 6:4  OC1M:  output compare 1 mode
// //                      100 - force low  (phases 3-4)
// //                      101 - force high (phases 2, 5)
// //                      110 - PWM mode 1 (phases 0-1)
// //    bit  3    OC1PE: output compare 1 preload enable (all phases)
// // CCMR2 - Capture/Compare Mode Register 2
// //    bit 14:12 OC4M:  output compare 2 mode
// //                      110 - PWM mode 1 (all phases)
// //    bit 11    OC4PE: output compare 2 preload enable (all phases)
// //    bit  7    OC3CE: OC1Ref cleared immediately on ETRF high (all phases)
// //    bits 6:4  OC3M:  output compare 1 mode
// //                      100 - force low  (phases 1-2)
// //                      101 - force high (phases 0, 3)
// //                      110 - PWM mode 1 (phases 4-5)
// //    bit  3    OC3PE: output compare 1 preload enable (all phases)
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

void init_timer(const timer *tp)
{
    const timer_periph *tpp = tp->periph;
    uint32_t tim = tpp->base;
    for (size_t i = 0; i < tpp->out_channel_count; i++) {
        assert(i == tpp->out_channels[i].id);
    }

    rcc_periph_clock_enable(tpp->clock);
    for (size_t i = 0; i < tpp->out_channel_count; i++) {
        const timer_oc *op = &tpp->out_channels[i];
        if (tp->enable_outputs & (1 << op->id))
            gpio_init_pin(&op->gpio);
    }

    timer_set_period(tim, timer_period(tp));

    for (size_t i = 0; i < tpp->out_channel_count; i++) {
        const timer_oc *op = &tpp->out_channels[i];
        if (tp->enable_outputs & (1 << op->id)) {
            timer_enable_oc_preload(tim, op->id);
            timer_enable_oc_output(tim, op->id);
        }
    }

    timer_enable_break_main_output(tim);
    timer_enable_counter(tim);
    // if (cfg->enable_LED) {
    //     enum tim_oc_id oc = TARGET_timer.LED_oc_id;
    //     timer_set_oc_mode(tim, oc, TIM_OCM_PWM1);
    //     timer_set_oc_polarity_low(tim, oc);
    //     timer_enable_oc_preload(tim, oc);
    //     timer_enable_oc_output(tim, oc);
    // }
    // if (cfg->enable_motor) {
    //     for (size_t i = 0; i < cfg->motor_pin_count; i++)
    //     // oc_mode at commutation
    //     // oc_polarity at commutation
    //     // oc_preload true
    //     // oc_output true
    // }
    // timer_enable_break_main_output(TIM1);
    // timer_enable_counter(tim);
}

uint16_t timer_period(const timer *tp)
{
    uint32_t period = rcc_apb1_frequency / tp->pwm_freq;
    assert(period < 65536);
    return period;
}

void timer_force_output_high(const timer *tp, enum tim_oc_id oc)
{
    timer_set_oc_mode(tp->periph->base, oc, TIM_OCM_FORCE_HIGH);
}

void timer_force_output_low(const timer *tp, enum tim_oc_id oc)
{
    timer_set_oc_mode(tp->periph->base, oc, TIM_OCM_FORCE_LOW);
}

void timer_enable_pwm(const timer *tp, enum tim_oc_id oc)
{
    timer_set_oc_mode(tp->periph->base, oc, TIM_OCM_PWM1);
}

void timer_set_pwm_duty(const timer *tp, enum tim_oc_id oc, uint16_t duty)
{
    timer_set_oc_value(tp->periph->base, oc, duty);
}

// void timer_set_LED_duty(uint16_t duty)
// {
//     timer_set_oc_value(TARGET_timer.base, TIM_OC4, duty);
// }
