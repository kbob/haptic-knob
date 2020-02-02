#ifndef PRINTF_included
#define PRINTF_included

extern int ee_printf(const char *__restrict, ...)
            __attribute__ ((__format__ (__printf__, 1, 2)));

#define printf ee_printf

#endif /* !PRINTF_included */
