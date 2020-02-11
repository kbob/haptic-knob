#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/rcc.h>

#include "intr.h"
#include "printf.h"
#include "usart.h"
#include TARGET_H

#define HISTOGRAM_BINS 128

typedef struct histogram {
    uint32_t count[HISTOGRAM_BINS];
    uint32_t value[HISTOGRAM_BINS];
} histogram;


static volatile uint32_t cc_counter;
static volatile uint32_t msec_counter;
static volatile uint32_t swint_counter;
static volatile uint32_t bg_counter;

static histogram cc_hist;
static histogram st_hist;
static histogram sw_hist;
static histogram bg_hist;

static const USART console_USART = {
    .periph         = &TARGET_USART,
    .baud           = 115200,
};

__attribute__((optimize("O0")))
static uint32_t timed_delay(uint32_t n)
{
    uint32_t nn = (n + (n >> 4)) >> 4;
    uint32_t before = STK_CVR;
    while (nn) {
        __asm__ volatile ( "nop" );
        --nn;
    }
    __asm__ volatile ( "nop" );
    __asm__ volatile ( "nop" );
    __asm__ volatile ( "nop" );
    uint32_t after = STK_CVR;
    uint32_t duration = before - after;
    if (duration > 4000000000UL)    // counter underflowed
        duration += 48001;
    return duration;
}

static void count_value(histogram *hp, uint32_t value)
{
    for (size_t i = 0; i < HISTOGRAM_BINS; i++) {
        if (hp->value[i] == value) {
            hp->count[i]++;
            return;
        } else if (!hp->count[i]) {
            hp->value[i] = value;
            hp->count[i] = 1;
            return;
        }
    }
    assert(0 && "histogram full");
}

static void atomic_copy_histogram(const histogram *src, histogram *dst)
{
    size_t i;
    WITH_INTERRUPTS_MASKED {
        for (i = 0; i < HISTOGRAM_BINS && src->count[i]; i++) {
            dst->value[i] = src->value[i];
            dst->count[i] = src->count[i];
        }
    }
    for ( ; i < HISTOGRAM_BINS; i++) {
        dst->value[i] = 0;
        dst->count[i] = 0;
    }
}

static size_t sort_histogram(histogram *hp)
{
    size_t n;
    for (n = 0; n < HISTOGRAM_BINS && hp->count[n]; n++)
        continue;
    for (size_t i = 0; i < n; i++) {
        for (size_t j = i + 1; j < n; j++) {
            if (hp->value[i] > hp->value[j]) {
                uint32_t tmp = hp->value[i];
                hp->value[i] = hp->value[j];
                hp->value[j] = tmp;
                tmp = hp->count[i];
                hp->count[i] = hp->count[j];
                hp->count[j] = tmp;
            }
        }
    }
    return n;
}

static uint32_t nl(void)
{
    printf("\33[K\n");
    return 1;
}

static uint32_t print_histogram(histogram *hp, size_t nbins, const char *label)
{
    uint32_t lines = 0;
    printf("%s", label);
    lines += nl();
    printf("     Cycles Count  Cycles Count  Cycles Count  Cycles Count");
    lines += nl();
    for (size_t i = 0; i < nbins; i++) {
        if (i % 4 == 0)
            printf("   ");
        printf(" %5lu: %6lu", hp->value[i], hp->count[i]);
        if ((i + 1) % 4 == 0)
            lines += nl();
    }
    if (nbins % 4)
        lines += nl();
    lines += nl();
    return lines;
}

extern void tim1_cc_isr(void)
{
    timer_clear_flag(TIM1, TIM_SR_CC1IF);
    // There does not appear to be a libopencm3 function to trigger
    // a soft interrupt.  So we poke the register directly.
    EXTI_SWIER |= EXTI2;
    uint32_t ticks = timed_delay(30);
    assert(ticks == 30);
    count_value(&cc_hist, ticks);
    cc_counter++;
}

void sys_tick_handler(void)
{
    uint32_t ticks = timed_delay(200);
    count_value(&st_hist, ticks);
    msec_counter++;
}

// software interrupt service routine
extern void exti2_3_isr(void)
{
    exti_reset_request(EXTI2);
    uint32_t ticks = timed_delay(500);
    count_value(&sw_hist, ticks);
    ++swint_counter;
    assert(swint_counter != 1000000);
}

int main(void)
{
    rcc_clock_setup_in_hse_8mhz_out_48mhz();
    init_USART(&console_USART);
    printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    printf("rcc_ahb_frequency = %lu\n", rcc_ahb_frequency);
    printf("rcc_apb1_frequency = %lu\n", rcc_apb1_frequency);

    // Set interrupt priorities.  From highest to lowest:
    //   timer CC
    //   systick
    //   other intrs (N/A)
    //   sw intr
    //   background
    //
    nvic_set_priority(NVIC_TIM1_CC_IRQ, 0x00);
    nvic_set_priority(NVIC_SYSTICK_IRQ, 0x40);
    // other intrs: 0x80
    nvic_set_priority(NVIC_EXTI2_3_IRQ, 0xC0);

    // Enable interrupts.
    nvic_enable_irq(NVIC_TIM1_CC_IRQ);
    nvic_enable_irq(NVIC_SYSTICK_IRQ);
    nvic_enable_irq(NVIC_EXTI2_3_IRQ);

    // Enable SW intr
    exti_enable_request(EXTI2);

    // Enable systick
    systick_set_reload(rcc_ahb_frequency / 1000);       // 1 KHz
    systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);
    systick_counter_enable();
    systick_interrupt_enable();

    // Enable timer
    rcc_periph_clock_enable(RCC_TIM1);
    timer_set_period(TIM1, rcc_apb1_frequency / 40000); // 40 KHz
    timer_enable_oc_preload(TIM1, TIM_OC1);
    timer_set_oc_mode(TIM1, TIM_OC1, TIM_OCM_PWM1);
    timer_set_oc_value(TIM1, TIM_OC1, 600);
    timer_enable_irq(TIM1, TIM_DIER_CC1IE);
    timer_enable_break_main_output(TIM1);
    timer_enable_counter(TIM1);

    uint32_t lines = 0;
    printf("\n");
    while (msec_counter < 10000) {
        // for (uint32_t i = 0; i < lines; i++)
        //     printf("\33[A");
        // lines = 0;
        // histogram h;
        // atomic_copy_histogram(&cc_hist, &h);
        // size_t nbins = sort_histogram(&h);
        // lines += print_histogram(&h, nbins, "Timer Int Timing");
        // atomic_copy_histogram(&st_hist, &h);
        // nbins = sort_histogram(&h);
        // lines += print_histogram(&h, nbins, "Systick Timing");
        // atomic_copy_histogram(&sw_hist, &h);
        // nbins = sort_histogram(&h);
        // lines += print_histogram(&h, nbins, "Soft Int Timing");
        // atomic_copy_histogram(&bg_hist, &h);
        // nbins = sort_histogram(&h);
        // lines += print_histogram(&h, nbins, "Background Timing");

        uint32_t ticks = timed_delay(100);
        count_value(&bg_hist, ticks);
        bg_counter++;
    }

    nvic_disable_irq(NVIC_TIM1_CC_IRQ);
    nvic_disable_irq(NVIC_SYSTICK_IRQ);
    nvic_disable_irq(NVIC_EXTI2_3_IRQ);

    lines = 0;
    histogram h;
    atomic_copy_histogram(&cc_hist, &h);
    size_t nbins = sort_histogram(&h);
    lines += print_histogram(&h, nbins, "Timer Int Timing");
    atomic_copy_histogram(&st_hist, &h);
    nbins = sort_histogram(&h);
    lines += print_histogram(&h, nbins, "Systick Timing");
    atomic_copy_histogram(&sw_hist, &h);
    nbins = sort_histogram(&h);
    lines += print_histogram(&h, nbins, "Soft Int Timing");
    atomic_copy_histogram(&bg_hist, &h);
    nbins = sort_histogram(&h);
    lines += print_histogram(&h, nbins, "Background Timing");
}

void __assert_func(const char *file, int line, const char *fn, const char *msg)
{
    printf("Assertion failed: %s, function %s, file %s, line %d\n",
           msg, fn, file, line);
    abort();
}
