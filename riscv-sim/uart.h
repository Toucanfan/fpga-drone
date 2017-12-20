#ifndef RVSIM_UART_H
#define RVSIM_UART_H 1

#include <stdint.h>

enum {
	UART_CSR,
	UART_DATA,
};

extern void uart_init(void);
extern void uart_update_state(void);
extern uint8_t uart_get_register(uint32_t reg);
extern void uart_set_register(uint32_t reg, uint8_t value);

#endif /* RVSIM_UART_H */
