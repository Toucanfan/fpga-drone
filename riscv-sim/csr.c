#include "csr.h"

typedef int csrset_f(uint16_t addr, uint32_t value);
typedef int csrget_f(uint16_t addr, uint32_t *value);

struct csr_desc {
	char *name;
	uint16_t addr; /* 12 bit address */
	csrget_f *do_get;
	csrset_f *do_set;
};

static int set_nothing(uint16_t addr, uint32_t value) {
	return 0;
}

static int get_zero(uint16_t addr, uint32_t *value) {
	return 0;
}

#define N_CSR 2
static struct csr_desc csr[N_CSR] = {
	{
		.name = "mstatus",
		.addr = 0x300,
		.do_get = NULL,
		.do_set = NULL,
	},
	{
		.name = "misa",
		.addr = 0x301,
		.do_get = get_zero,
		.do_set = set_nothing,
	},
	{
		.name = "mvendorid",
		.addr = 0xF11,
		.do_get = get_zero,
		.do_set = set_nothing,
	},
	{
		.name = "marchid",
		.addr = 0xF12,
		.do_get = get_zero,
		.do_set = set_nothing,
	},
	{
		.name = "mimpid",
		.addr = 0xF13,
		.do_get = get_zero,
		.do_set = set_nothing,
	},
	{
		.name = "mhartid",
		.addr = 0xF14,
		.do_get = get_zero,
		.do_set = set_nothing,
	},
};

int csr_set(uint16_t reg, uint32_t value) {
	for (int i = 0; i < N_CSR; ++i) {
		struct csr_desc c = csr[i];
		if (c.do_set)
			return c.do_set(reg, value);
		else
			return -EINVAL;
	}
	return -EINVAL;
}

int csr_get(uint16_t reg, uint32_t *value) {
	for (int i = 0; i < N_CSR; ++i) {
		struct csr_desc c = csr[i];
		if (c.do_get)
			return c.do_get(reg, value);
		else
			return -EINVAL;
	}
	return -EINVAL;
}
