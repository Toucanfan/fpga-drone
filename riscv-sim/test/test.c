#include "uart.h"

char msg[] = "Hello world!\r\n";

int main(int argc, char **argv) {
	char c;
	uart_puts("Hello world!\r\n");
	uart_puts("Echo mode is on!\r\n");
	for (;;) {
		c = uart_read();
		if (c == '\r')
			uart_puts("\r\n");
		else
			uart_write(c);
	}
	return 0;
}
