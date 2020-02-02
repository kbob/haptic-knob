#ifndef USART_included
#define USART_included

extern void init_USART(int baud);
extern void USART_init_stdio(void);

extern void USART_puts(const char *);
extern void USART_putchar(int);

extern void USART_putstr(const char *);
extern void USART_putdec(int);
extern void USART_puthex(unsigned);
extern void USART_putbin(unsigned);

#endif /* !USART_included */
