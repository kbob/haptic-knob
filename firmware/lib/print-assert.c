#include <assert.h>
#include <stdlib.h>

#include <libopencm3/cm3/cortex.h>

#include "printf.h"

void __assert_func(const char *file, int line, const char *fn, const char *msg)
{
    cm_disable_interrupts();
    printf("Assertion failed: %s, function %s, file %s, line %d\n",
           msg, fn, file, line);
    abort();
}
