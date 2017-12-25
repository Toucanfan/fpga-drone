#include "uart.h"

#define UART_BASE 0x80000000
#define UART_CSR (*(volatile char *)(UART_BASE))
#define UART_DATA (*(volatile char *)(UART_BASE + 1))
#define CSR_NEWDAT 0x1
#define CSR_TXDONE 0x2

void uart_write(char c) {
	while (!(UART_CSR & CSR_TXDONE));
	UART_DATA = c;
}

char uart_read(void) {
	while (!(UART_CSR & CSR_NEWDAT));
	return UART_DATA;
}

void uart_puts(char *str) {
	while (*str != '\0') {
		uart_write(*str++);
	}
}
