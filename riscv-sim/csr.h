#ifndef RVSIM_CSR_H
#define RVSIM_CSR_H 1

enum {
	/* User Counter/Timers */ /* not needed */
	CSR_CYCLE      = 0xC00, /* URO */
	CSR_TIME       = 0xC01, /* URO */
	CSR_INSTRET    = 0xC02, /* URO */
	CSR_CYCLEH     = 0xC80, /* URO */
	CSR_TIMEH      = 0xC81, /* URO */
	CSR_INSTRETH   = 0xC82, /* URO */

	/* Machine Information Registers */
	CSR_MVENDORID  = 0xF11, /* MRO */ /* return 0 if not implemented */
	CSR_MARCHID    = 0xF12, /* MRO */ /* return 0 if not implemented */
	CSR_MIMPID     = 0xF13, /* MRO */ /* return 0 if not implemented */
	CSR_MHARTID    = 0xF14, /* MRO */ /* return 0 - single core system */

	/* Machine Trap Setup */
	CSR_MSTATUS    = 0x300, /* MRW */
	CSR_MISA       = 0x301, /* MRW */ /* return 0 if not implemented */
	CSR_MIE        = 0x304, /* MRW */
	CSR_MTVEC      = 0x305, /* MRW */
	CSR_MCOUNTEREN = 0x306, /* MRW */

	/* Machine Trap Handling */
	CSR_MSCRATCH   = 0x340, /* MRW */
	CSR_MEPC       = 0x341, /* MRW */
	CSR_MCAUSE     = 0x342, /* MRW */
	CSR_MTVAL      = 0x343, /* MRW */
	CSR_MIP        = 0x344, /* MRW */

	/* Machine Protection and Translation */
	CSR_PMPCFG0    = 0x3A0, /* MRW */
	CSR_PMPCFG1    = 0x3A1, /* MRW */
	CSR_PMPCFG2    = 0x3A2, /* MRW */
	CSR_PMPCFG3    = 0x3A3, /* MRW */
	CSR_PMPADDR0   = 0x3B0, /* MRW */
	CSR_PMPADDR1   = 0x3B1, /* MRW */
	CSR_PMPADDR2   = 0x3B2, /* MRW */
	CSR_PMPADDR3   = 0x3B3, /* MRW */
	CSR_PMPADDR4   = 0x3B4, /* MRW */
	CSR_PMPADDR5   = 0x3B5, /* MRW */
	CSR_PMPADDR6   = 0x3B6, /* MRW */
	CSR_PMPADDR7   = 0x3B7, /* MRW */
	CSR_PMPADDR8   = 0x3B8, /* MRW */
	CSR_PMPADDR9   = 0x3B9, /* MRW */
	CSR_PMPADDR10  = 0x3BA, /* MRW */
	CSR_PMPADDR11  = 0x3BB, /* MRW */
	CSR_PMPADDR12  = 0x3BC, /* MRW */
	CSR_PMPADDR13  = 0x3BD, /* MRW */
	CSR_PMPADDR14  = 0x3BE, /* MRW */
	CSR_PMPADDR15  = 0x3BF, /* MRW */

	/* Machine Counter/Timers */
	CSR_MCYCLE     = 0xB00, /* MRW */
	CSR_MINSTRET   = 0xB02, /* MRW */
	CSR_MCYCLEH    = 0xB80, /* MRW */
	CSR_MINSTRETH  = 0xB82, /* MRW */
};

extern int csr_set(uint16_t reg, uint32_t value);
extern int csr_get(uint16_t reg, uint32_t *value);

#endif /* RVSIM_CSR_H */
