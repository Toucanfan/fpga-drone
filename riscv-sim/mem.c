#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "mem.h"
#include "uart.h"

#define ROM_SIZE (1 << 5)
#define RAM_SIZE (1 << 5)


typedef int storefunc_t(uint32_t addr, uint8_t value);
typedef int loadfunc_t(uint32_t addr, uint8_t *value);

struct mem_region {
	char *name;
	uint32_t s_addr;
	uint32_t e_addr;
	storefunc_t *do_store;
	loadfunc_t *do_load;
};

static uint8_t *internal_ram = NULL;
static uint8_t *internal_rom = NULL;

static int store_ram(uint32_t addr, uint8_t value) {
	internal_ram[addr & 0xffff] = value;
	return 0;
}

static int load_ram(uint32_t addr, uint8_t *value) {
	*value = internal_ram[addr & 0xffff];
	return 0;
}

static int load_rom(uint32_t addr, uint8_t *value) {
	*value = internal_rom[addr & 0xffff];
	return 0;
}

static int load_uart(uint32_t addr, uint8_t *value) {
	if (addr & 0x1)
		*value = uart_get_register(UART_DATA);
	else
		*value = uart_get_register(UART_CSR);
	return 0;
}

static int store_uart(uint32_t addr, uint8_t value) {
	if (addr & 0x1)
		uart_set_register(UART_DATA, value);
	else
		uart_set_register(UART_CSR, value);
	return 0;
}


#define N_MEMREGIONS 3
static struct mem_region mem_space[N_MEMREGIONS] = {
	{
		.name = "Internal ROM",
		.s_addr = 0x00000000,
		.e_addr = 0x0000ffff,
		.do_store = NULL,
		.do_load = load_rom,
	},
	{
		.name = "Internal RAM",
		.s_addr = 0x00010000,
		.e_addr = 0x0001ffff,
		.do_store = store_ram,
		.do_load = load_ram,
	},
	{
		.name = "UART",
		.s_addr = 0x80000000,
		.e_addr = 0x80000001,
		.do_store = store_uart,
		.do_load = load_uart,
	},

};


int mem_store_byte(uint32_t addr, uint8_t value) {
	for (int i = 0; i < N_MEMREGIONS; ++i) {
		struct mem_region r = mem_space[i];
		if ((addr >= r.s_addr) && (addr <= r.e_addr)) {
			if (r.do_store) {
				return r.do_store(addr, value);
			} else {
				return -EACCES;
			}
		}
	}
	return -EINVAL;
}

int mem_load_byte(uint32_t addr, uint8_t *value) {
	for (int i = 0; i < N_MEMREGIONS; ++i) {
		struct mem_region r = mem_space[i];
		if ((addr >= r.s_addr) && (addr <= r.e_addr)) {
			if (r.do_load)
				return r.do_load(addr, value);
			else
				return -EACCES;
		}
	}
	return -EINVAL;
}

void mem_init(void) {
	internal_rom = malloc(ROM_SIZE);
	if (internal_rom == NULL)
		goto fail;
	internal_ram = malloc(RAM_SIZE);
	if (internal_ram == NULL)
		goto fail;

	memset(internal_rom, 0, ROM_SIZE);
	memset(internal_ram, 0, RAM_SIZE);
	return;

fail:
	perror("malloc");
	exit(EXIT_FAILURE);
}

void mem_rom_load_flatbin(uint32_t offset, char *filepath) {
	int fd = open(filepath, O_RDONLY);
	if (fd < 0) {
		perror("open");
		exit(EXIT_FAILURE);
	}

	struct stat st;
	if (fstat(fd, &st) < 0) {
		perror("fstat");
		exit(EXIT_FAILURE);
	}
	int programSize = st.st_size;
	if ((programSize + offset) < ROM_SIZE) {
		fprintf(stderr, "mem_rom_load_flatbin: %s too large\n", filepath);
		exit(EXIT_FAILURE);
	}

	uint8_t *program = mmap(NULL, programSize, PROT_READ, MAP_PRIVATE, fd, 0);
	if (program == MAP_FAILED) {
		perror("mmap");
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < programSize; ++i) {
		internal_rom[offset + i] = program[i];
	}
}
