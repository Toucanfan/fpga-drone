#ifndef RVSIM_UART_H
#define RVSIM_UART_H 1

#include <stdint.h>

enum {
	UART_CSR,
	UART_DATA,
};

extern uint32_t uart_get_register(uint32_t reg);
extern void uart_set_register(uint32_t reg, uint32_t value);

#endif /* RVSIM_UART_H */
