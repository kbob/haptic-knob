#ifndef REGDUMP_included
#define REGDUMP_included

#include <stdint.h>

extern void rewind_advanced_timer_registers(void);

extern void dump_advanced_timer_registers(uint32_t base);

#endif /* !REGDUMP_included */
