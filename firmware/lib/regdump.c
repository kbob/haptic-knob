#include "regdump.h"

#include <stdbool.h>
#include <stddef.h>

#include <libopencm3/stm32/timer.h>

#include "printf.h"

#define REG_PER_LINE 5
#define LINE_COUNT(n) (((n) + REG_PER_LINE - 1)  / REG_PER_LINE)
#define ARRAY_COUNT(a) ((&(a))[1] - (a))

#define UP   "\33[A"            // ANSI move cursor up
#define CLNL "\33[K\n"          // ANSI clear to end of line, newline

typedef struct reg {
    char     name[7];
    uint8_t offset;
} reg;

#define DEF_REG(name) { #name, (uint32_t)&TIM_##name(0) }

static const reg advanced_timer_regs[] = {
    DEF_REG(CR1),
    DEF_REG(CR2),
    DEF_REG(SMCR),
    DEF_REG(DIER),
    DEF_REG(SR),
    DEF_REG(EGR),
    DEF_REG(CCMR1),
    DEF_REG(CCMR2),
    DEF_REG(CCER),
    DEF_REG(CNT),
    DEF_REG(PSC),
    DEF_REG(ARR),
    DEF_REG(RCR),
    DEF_REG(CCR1),
    DEF_REG(CCR2),
    DEF_REG(CCR3),
    DEF_REG(CCR4),
    DEF_REG(BDTR),
    DEF_REG(DCR),
    DEF_REG(DMAR),
};
static const size_t advanced_timer_reg_count = ARRAY_COUNT(advanced_timer_regs);

static void dump_periph_regs(uint32_t base, const reg regs[], size_t count)
{
    size_t rows = LINE_COUNT(count);
    for (size_t i = 0; i < rows; i++) {
        printf("  ");
        for (size_t j = i; j < count; j += rows) {
            printf("  %-6s %4lx", regs[j].name, MMIO32(base + regs[j].offset));
        }
        printf(CLNL);
    }
}

void rewind_advanced_timer_registers(void)
{
    static bool been_here;
    if (!been_here) {
        been_here = true;
    } else {
        for (size_t i = 0; i < LINE_COUNT(advanced_timer_reg_count); i++)
            printf(UP);
    }
}

void dump_advanced_timer_registers(uint32_t base)
{
    dump_periph_regs(base, advanced_timer_regs, advanced_timer_reg_count);
}
