#define _GNU_SOURCE

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#include "uart.h"

#define CSR_NEWDAT (1 << 0)
#define CSR_TXDONE (1 << 1)

static struct uart_regs {
	uint8_t data_r;
	uint8_t data_w; /* data_w is shadow register */
	uint8_t csr;
} regs;

static int ptm = -1;

void uart_update_state(void) {
	if (!(regs.csr & CSR_NEWDAT)) {
		uint8_t buf[32];
		if (read(ptm, buf, 1) == 1) {
			regs.data_r = buf[0];
			regs.csr |= CSR_NEWDAT;
		}
		while(read(ptm, buf, sizeof buf) > 0); /* flush input buffer */
	}
	if (!(regs.csr & CSR_TXDONE)) {
		write(ptm, &regs.data_w, 1);
		regs.csr |= CSR_TXDONE;
	}
}

uint8_t uart_get_register(uint32_t reg) {
	switch (reg) {
	case UART_DATA:
		regs.csr &= ~CSR_NEWDAT;
		return regs.data_r;
		break;
	case UART_CSR:
		return regs.csr;
		break;
	default:
		fprintf(stderr, "uart_get_register: register unknown\n");
		abort();
		break;
	}
}

void uart_set_register(uint32_t reg, uint8_t value) {
	switch (reg) {
	case UART_DATA:
		regs.data_w = value;
		regs.csr &= ~CSR_TXDONE;
		break;
	case UART_CSR:
		/* do nothing for now */
		break;
	default:
		fprintf(stderr, "uart_set_register: register unknown\n");
		abort();
		break;
	}
}
	
void uart_init(void) {
	ptm = getpt();
	if (ptm < 0) {
		perror("getpt");
		exit(EXIT_FAILURE);
	}

	if (grantpt(ptm) < 0) {
		perror("grantpt");
		exit(EXIT_FAILURE);
	}
	if (unlockpt(ptm) < 0) {
		perror("unlockpt");
		exit(EXIT_FAILURE);
	}

	struct termios t;
	if (tcgetattr(ptm, &t) < 0) {
		perror("tcgetattr");
		exit(EXIT_FAILURE);
	}
	cfmakeraw(&t);
	if (tcsetattr(ptm, TCSANOW, &t) < 0) {
		perror("tcsetattr");
		exit(EXIT_FAILURE);
	}

	printf("serial port: %s\n", ptsname(ptm));
}
