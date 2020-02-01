#ifndef LED_included
#define LED_included

#include <stdbool.h>

extern void init_LED(void);

extern void LED_on(void);
extern void LED_off(void);
extern void LED_set(bool on_off);
extern void LED_toggle(void);

#endif /* !LED_included */
