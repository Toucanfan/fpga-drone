#ifndef RVTEST_UART_H
#define RVTEST_UART_H 1

extern void uart_write(char c);
extern char uart_read(void);
extern void uart_puts(char *str);

#endif /* RVTEST_UART_H */
