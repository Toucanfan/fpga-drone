#define _GNU_SOURCE 1

#include <fcntl.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>

#include "mem.h"
#include "uart.h"

#define MEMBIT 24

#define OP_LOAD    0x00 /* 00000 */
#define OP_IMM     0x04 /* 00100 */
#define OP_AUIPC   0x05 /* 00101 */
#define OP_STORE   0x08 /* 01000 */
#define OP         0x0C /* 01100 */
#define OP_LUI     0x0D /* 01101 */
#define OP_BRANCH  0x18 /* 11000 */
#define OP_JALR    0x19 /* 11001 */
#define OP_JAL     0x1B /* 11011 */
#define OP_SYSTEM  0x1C /* 11100 */

static struct arguments {
	bool verbose;
	bool single_step;
	bool print_regs;
	bool enable_uart;
	char *bin_file;
} args;

struct machine {
	uint32_t pc;
	uint32_t regs[32];
} M;

enum {
	E_INVAL_INSTR = 0,
	E_SOME_CRAP,
};

static uint32_t get_bit31(const uint32_t instr) {
	return (instr >> 31) & 0x01; /* 1 */
	return 0;
}

static uint32_t sign_extend(uint32_t imm, int signPos) {
	if (imm & (1 << signPos)) {
		for (int i = signPos; i < 32; ++i) {
			imm |= (1 << i);
		}
	}
	return imm;
}
			

static uint32_t get_op(const uint32_t instr) {
	return (instr >> 2) & 0x1F; /* 1 1111 */
}

static uint32_t get_rd(const uint32_t instr) {
	return (instr >> 7) & 0x1F; /* 1 1111 */
}

static uint32_t get_funct3(const uint32_t instr) {
	return (instr >> 12) & 0x07; /* 111 */
}

static uint32_t get_funct7(const uint32_t instr) {
	return (instr >> 25) & 0x7F; /* 111 1111 */
}

static uint32_t get_rs1(const uint32_t instr) {
	return (instr >> 15) & 0x1F; /* 1 1111 */
}

static uint32_t get_rs2(const uint32_t instr) {
	return (instr >> 20) & 0x1F; /* 1 1111 */
}

static uint32_t get_i_imm(const uint32_t instr) {
	return (instr >> 20) & 0xFFF; /* 1111 1111 1111 */
}

static uint32_t get_s_imm(const uint32_t instr) {
	uint32_t imm_11_5 = (instr >> 25) & 0x7F; /* 111 1111 */
	uint32_t imm_4_0 = (instr >> 7) & 0x1F; /* 1 1111 */
	return (imm_11_5 << 5) | imm_4_0;
}

static uint32_t get_b_imm(const uint32_t instr) {
	uint32_t imm_12 = (instr >> 31) & 0x01; /* 1 */
	uint32_t imm_10_5 = (instr >> 25) & 0x3F; /* 11 1111 */
	uint32_t imm_4_1 = (instr >> 8) & 0x0F; /* 1111 */
	uint32_t imm_11 = (instr >> 7) & 0x01; /* 1 */
	return (imm_12 << 12)|(imm_11 << 11)|(imm_10_5 << 5)|(imm_4_1 << 1);
}

static uint32_t get_u_imm(const uint32_t instr) {
	return (instr & 0xFFFFF000) >> 12; /* 1111 1111 1111 1111 1111 0000 0000 0000 */
}

static uint32_t get_j_imm(const uint32_t instr) {
	uint32_t imm_20 = (instr >> 31) & 0x01; /* 1 */
	uint32_t imm_10_1 = (instr >> 21) & 0x3FF; /* 11 1111 1111 */
	uint32_t imm_11 = (instr >> 20) & 0x01; /* 1 */
	uint32_t imm_19_12 = (instr >> 12) & 0xFF; /* 1111 1111 */
	return (imm_20 << 20)|(imm_19_12 << 12)|(imm_11 << 11)|(imm_10_1 << 1);
}

static void verbose_printf(const char *format, ...) {
	va_list ap;
	va_start(ap, format);

	if (args.verbose)
		vprintf(format, ap);
}

enum {
	REG_ZERO = 0,
	REG_RA   = 1,
	REG_SP   = 2,
	REG_GP   = 3,
	REG_TP   = 4,
	REG_T0   = 5,
	REG_T1   = 6,
	REG_T2   = 7,
	REG_S0   = 8,
	REG_S1   = 9,
	REG_A0   = 10,
	REG_A1   = 11,
	REG_A2   = 12,
	REG_A3   = 13,
	REG_A4   = 14,
	REG_A5   = 15,
	REG_A6   = 16,
	REG_A7   = 17,
	REG_S2   = 18,
	REG_S3   = 19,
	REG_S4   = 20,
	REG_S5   = 21,
	REG_S6   = 22,
	REG_S7   = 23,
	REG_S8   = 24,
	REG_S9   = 25,
	REG_S10  = 26,
	REG_S11  = 27,
	REG_T3   = 28,
	REG_T4   = 29,
	REG_T5   = 30,
	REG_T6   = 31,
};

static const char *const regname[] = {
	[REG_ZERO] = "zero",
	[REG_RA]   = "ra",
	[REG_SP]   = "sp",
	[REG_GP]   = "gp",
	[REG_TP]   = "tp",
	[REG_T0]   = "t0",
	[REG_T1]   = "t1",
	[REG_T2]   = "t2",
	[REG_S0]   = "s0",
	[REG_S1]   = "s1",
	[REG_A0]   = "a0",
	[REG_A1]   = "a1",
	[REG_A2]   = "a2",
	[REG_A3]   = "a3",
	[REG_A4]   = "a4",
	[REG_A5]   = "a5",
	[REG_A6]   = "a6",
	[REG_A7]   = "a7",
	[REG_S2]   = "s2",
	[REG_S3]   = "s3",
	[REG_S4]   = "s4",
	[REG_S5]   = "s5",
	[REG_S6]   = "s6",
	[REG_S7]   = "s7",
	[REG_S8]   = "s8",
	[REG_S9]   = "s9",
	[REG_S10]  = "s10",
	[REG_S11]  = "s11",
	[REG_T3]   = "t3",
	[REG_T4]   = "t4",
	[REG_T5]   = "t5",
	[REG_T6]   = "t6",
};
	

static void trap_invalid_instr(void) {
	verbose_printf("illegal instruction ");
}

static void trap_invalid_memory(void) {
	verbose_printf("illegal memory access ");
}

static void trap_illegal_alignment(void) {
	verbose_printf("unaligned memory access ");
}

union memdata {
	uint32_t word;
	uint16_t hwords[2];
	uint8_t bytes[4];
};

static void exec_op_load(uint32_t instr) {
	uint32_t rd = get_rd(instr);
	uint32_t rs1 = get_rs1(instr);
	uint32_t offset = sign_extend(get_i_imm(instr), 11);
	uint32_t eff = M.regs[rs1] + offset;

	union memdata m = {0};
	int r = -1;

	switch(get_funct3(instr)) {
	case 0: /* 000 LB */
		r = mem_load_byte(eff, &m.bytes[0]);
		if (r < 0) {
			trap_invalid_memory();
			return;
		}
		M.regs[rd] = sign_extend((uint32_t)m.bytes[0], 7);
		verbose_printf("lb %s,%d(%s) ", regname[rd], (int32_t)offset, regname[rs1]);
		break;
	
	case 1: /* 001 LH */
		if (eff % 2) {
			trap_illegal_alignment();
			return;
		}
		for (int i = 0; i < 2; ++i) {
			r = mem_load_byte(eff + i, &m.bytes[i]);
			if (r < 0) {
				trap_invalid_memory();
				return;
			}
		}
		M.regs[rd] = sign_extend((uint32_t)m.hwords[0], 15);
		verbose_printf("lh %s,%d(%s) ", regname[rd], (int32_t)offset, regname[rs1]);
		break;

	case 2: /* 010 LW */
		if (eff % 4) {
			trap_illegal_alignment();
			return;
		}
		for (int i = 0; i < 4; ++i) {
			r = mem_load_byte(eff + i, &m.bytes[i]);
			if (r < 0) {
				trap_invalid_memory();
				return;
			}
		}
		M.regs[rd] = m.word;
		verbose_printf("lw %s,%d(%s) ", regname[rd], (int32_t)offset, regname[rs1]);
		break;

	case 4: /* 100 LBU */
		r = mem_load_byte(eff, &m.bytes[0]);
		if (r < 0) {
			trap_invalid_memory();
			return;
		}
		M.regs[rd] = (uint32_t)m.bytes[0];
		verbose_printf("lbu %s,%d(%s) ", regname[rd], (int32_t)offset, regname[rs1]);
		break;

	case 5: /* 101 LHU */
		if (eff % 2) {
			trap_illegal_alignment();
			return;
		}
		for (int i = 0; i < 2; ++i) {
			r = mem_load_byte(eff + i, &m.bytes[i]);
			if (r < 0) {
				trap_invalid_memory();
				return;
			}
		}
		M.regs[rd] = (uint32_t)m.hwords[0];
		verbose_printf("lhu %s,%d(%s) ", regname[rd], (int32_t)offset, regname[rs1]);
		break;
		
	default:
		trap_invalid_instr();
		break;
	}

	/* printf("OP_LOAD "); */
}

static void exec_op_imm(uint32_t instr) {
	uint32_t rd = get_rd(instr);
	uint32_t rs1 = get_rs1(instr);
	uint32_t shamt = get_rs2(instr);
	uint32_t funct7 = get_funct7(instr);
	uint32_t imm = get_i_imm(instr);

	switch(get_funct3(instr)) {
	case 0: /* 000 ADDI */
		imm = sign_extend(imm, 11);
		M.regs[rd] = M.regs[rs1] + imm;
		verbose_printf("addi %s,%s,%d ", regname[rd], regname[rs1], imm);
		break;
	case 1: /* 001 */
		switch(funct7) {
		case 0x00: /* 000 0000 SLLI */
			M.regs[rd] = M.regs[rs1] << shamt;
			verbose_printf("slli %s,%s,%d ", regname[rd], regname[rs1], shamt);
			break;
		default:
			trap_invalid_instr();
			break;
		}
		break;
	case 2: /* 010 SLTI */
		imm = sign_extend(imm, 11);
		if ((int32_t)M.regs[rs1] < (int32_t)imm)
			M.regs[rd] = 1;
		else
			M.regs[rd] = 0;
		verbose_printf("slti %s,%s,%d ", regname[rd], regname[rs1], imm);
		break;
	case 3: /* 011 SLTIU */
		imm = sign_extend(imm, 11);
		if (M.regs[rs1] < imm)
			M.regs[rd] = 1;
		else
			M.regs[rd] = 0;
		verbose_printf("sltiu %s,%s,%d ", regname[rd], regname[rs1], imm);
		break;
	case 4: /* 100 XORI */
		imm = sign_extend(imm, 11);
		M.regs[rd] = M.regs[rs1] ^ imm;
		verbose_printf("xori %s,%s,%d ", regname[rd], regname[rs1], imm);
		break;
	case 5: /* 101 */
		switch(funct7) {
		case 0x00: /* 000 0000 SRLI */
			M.regs[rd] = M.regs[rs1] >> shamt;
			verbose_printf("srli %s,%s,%d ", regname[rd], regname[rs1], shamt);
			break;
		case 0x20: /* 010 0000 SRAI */
			M.regs[rd] = (uint32_t)(((int32_t)M.regs[rs1]) >> shamt);
			verbose_printf("srai %s,%s,%d ", regname[rd], regname[rs1], shamt);
			break;
		default:
			trap_invalid_instr();
			break;
		}
		break;
	case 6: /* 110 ORI */
		imm = sign_extend(imm, 11);
		M.regs[rd] = M.regs[rs1] | imm;
		verbose_printf("ori %s,%s,%d ", regname[rd], regname[rs1], imm);
		break;
	case 7: /* 111 ANDI */
		imm = sign_extend(imm, 11);
		M.regs[rd] = M.regs[rs1] & imm;
		verbose_printf("andi %s,%s,%d ", regname[rd], regname[rs1], imm);
		break;
	default:
		trap_invalid_instr();
		break;
	}
	/* verbose_printf("OP_IMM "); */
}

static void exec_op_auipc(uint32_t instr) {
	uint32_t rd = get_rd(instr);
	uint32_t offset = get_u_imm(instr) << 12;
	M.regs[rd] = M.pc + offset;
	verbose_printf("auipc %s,%u ", regname[rd], offset);
}

static void exec_op_store(uint32_t instr) {
	uint32_t rs1 = get_rs1(instr);
	uint32_t rs2 = get_rs2(instr);
	uint32_t offset = sign_extend(get_s_imm(instr), 11);
	uint32_t eff = M.regs[rs1] + offset;
	int n_bytes = 0;
	union memdata m = {0};
	m.word = M.regs[rs2];
	char str[128];

	switch(get_funct3(instr)) {
	case 0: /* 000 SB */
		n_bytes = 1;
		snprintf(str, sizeof str, "sb %s,%d(%s) ", regname[rs2], (int32_t)offset, regname[rs1]);
		break;
	case 1: /* 001 SH */
		if (eff % 2) {
			trap_illegal_alignment();
			return;
		}
		n_bytes = 2;
		snprintf(str, sizeof str, "sh %s,%d(%s) ", regname[rs2], (int32_t)offset, regname[rs1]);
		break;
	case 2: /* 010 SW */
		if (eff % 4) {
			trap_illegal_alignment();
			return;
		}
		n_bytes = 4;
		snprintf(str, sizeof str, "sw %s,%d(%s) ", regname[rs2], (int32_t)offset, regname[rs1]);
		break;
	default:
		trap_invalid_instr();
		return;
	}

	for (int i = 0; i < n_bytes; ++i) {
		int r = mem_store_byte(eff + i, m.bytes[i]);
		if (r < 0) {
			trap_invalid_memory();
			return;
		}
	}

	verbose_printf("%s", str);
}

static void exec_op(uint32_t instr) {
	uint32_t rd = get_rd(instr);
	uint32_t rs1 = get_rs1(instr);
	uint32_t rs2 = get_rs2(instr);
	uint32_t funct7 = get_funct7(instr);

	switch(get_funct3(instr)) {
	case 0: /* 000 */
		if (funct7 & 0x20) { /* 010 0000 SUB */
			M.regs[rd] = M.regs[rs1] - M.regs[rs2];
			verbose_printf("sub %s,%s,%s ", regname[rd], regname[rs1], regname[rs2]);
		} else { /* 000 0000 ADD */
			M.regs[rd] = M.regs[rs1] + M.regs[rs2];
			verbose_printf("add %s,%s,%s ", regname[rd], regname[rs1], regname[rs2]);
		}
		break;
	case 1: /* 001 SLL */
		M.regs[rd] = M.regs[rs1] << (M.regs[rs2] & 0x1F); /* 1 1111 */
		verbose_printf("sll %s,%s,%s ", regname[rd], regname[rs1], regname[rs2]);
		break;
	case 2: /* 010 SLT */
		if ((int32_t)M.regs[rs1] < (int32_t)M.regs[rs2])
			M.regs[rd] = 1;
		else
			M.regs[rd] = 0;
		verbose_printf("slt %s,%s,%s ", regname[rd], regname[rs1], regname[rs2]);
		break;
	case 3: /* 011 SLTU */
		if (M.regs[rs1] < M.regs[rs2])
			M.regs[rd] = 1;
		else
			M.regs[rd] = 0;
		verbose_printf("slu %s,%s,%s ", regname[rd], regname[rs1], regname[rs2]);
		break;
	case 4: /* 100 XOR */
		M.regs[rd] = M.regs[rs1] ^ M.regs[rs2];
		verbose_printf("xor %s,%s,%s ", regname[rd], regname[rs1], regname[rs2]);
		break;
	case 5: /* 101 */
		if (funct7 & 0x20) { /* 010 0000 SRA */
			M.regs[rd] = (int32_t)M.regs[rs1] >> (M.regs[rs2] & 0x1F);
			verbose_printf("sra %s,%s,%s ", regname[rd], regname[rs1], regname[rs2]);
		} else { /* 000 0000 SRL */
			M.regs[rd] = M.regs[rs1] >> (M.regs[rs2] & 0x1F);
			verbose_printf("srl %s,%s,%s ", regname[rd], regname[rs1], regname[rs2]);
		}
		break;
	case 6: /* 110 OR */
		M.regs[rd] = M.regs[rs1] | M.regs[rs2];
		verbose_printf("or %s,%s,%s ", regname[rd], regname[rs1], regname[rs2]);
		break;
	case 7: /* 111 AND */
		M.regs[rd] = M.regs[rs1] & M.regs[rs2];
		verbose_printf("and %s,%s,%s ", regname[rd], regname[rs1], regname[rs2]);
		break;
	default:
		trap_invalid_instr();
		break;
	}
}

static void exec_op_lui(uint32_t instr) {
	uint32_t rd = get_rd(instr);
	uint32_t imm = get_u_imm(instr) << 12;
	M.regs[rd] = imm;
	verbose_printf("lui %s,0x%x ", regname[rd], imm >> 12);
}

static void exec_op_branch(uint32_t instr) {
	uint32_t rs1 = get_rs1(instr);
	uint32_t rs2 = get_rs2(instr);
	uint32_t offset = sign_extend(get_b_imm(instr), 12);
	uint32_t eff = M.pc + offset;

	switch(get_funct3(instr)) {
	case 0: /* 000 BEQ */
		if (M.regs[rs1] == M.regs[rs2])
			M.pc = eff - 4;
		verbose_printf("beq %s,%s,0x%x ", regname[rs1], regname[rs2], eff);
		break;
	case 1: /* 001 BNE */
		if (M.regs[rs1] != M.regs[rs2])
			M.pc = eff - 4;
		verbose_printf("bne %s,%s,0x%x ", regname[rs1], regname[rs2], eff);
		break;
	case 4: /* 100 BLT */
		if ((int32_t)M.regs[rs1] < (int32_t)M.regs[rs2])
			M.pc = eff - 4;
		verbose_printf("blt %s,%s,0x%x ", regname[rs1], regname[rs2], eff);
		break;
	case 5: /* 101 BGE */
		if ((int32_t)M.regs[rs1] >= (int32_t)M.regs[rs2])
			M.pc = eff - 4;
		verbose_printf("bge %s,%s,0x%x ", regname[rs1], regname[rs2], eff);
		break;
	case 6: /* 110 BLTU */
		if (M.regs[rs1] < M.regs[rs2])
			M.pc = eff - 4;
		verbose_printf("bltu %s,%s,0x%x ", regname[rs1], regname[rs2], eff);
		break;
	case 7: /* 111 BGEU */
		if (M.regs[rs1] >= M.regs[rs2])
			M.pc = eff - 4;
		verbose_printf("bgeu %s,%s,0x%x ", regname[rs1], regname[rs2], eff);
		break;
	default:
		trap_invalid_instr();
		break;
	}
}

static void exec_op_jalr(uint32_t instr) {
	uint32_t rd = get_rd(instr);
	uint32_t rs1 = get_rs1(instr);
	uint32_t offset = sign_extend(get_i_imm(instr), 11);
	uint32_t eff = (M.regs[rs1] + offset) & ~0x1;
	M.regs[rd] = M.pc + 4;
	M.pc = eff - 4;
	verbose_printf("jalr %s,%s,0x%x", regname[rd], regname[rs1], eff);
}

static void exec_op_jal(uint32_t instr) {
	uint32_t rd = get_rd(instr);
	uint32_t offset = sign_extend(get_j_imm(instr), 20);
	uint32_t eff = M.pc + offset;
	M.regs[rd] = M.pc + 4;
	M.pc = eff - 4; /* take into account coming increment in this cycle */
	verbose_printf("jal %s,0x%x ", regname[rd], eff);
}

static void exec_op_system(uint32_t instr) {
	verbose_printf("OP_SYSTEM ");
}

static void run_machine_cycle(void) {
	/* Fetch */
	union memdata m = {0};
	for (int i = 0; i < 4; ++i) {
		int r = mem_load_byte(M.pc + i, &m.bytes[i]);
		if (r < 0) {
			trap_invalid_memory();
			return;
		}
	}
	uint32_t instr = m.word;

	verbose_printf("%.8x: %.8x - ", M.pc, instr);
	
	/* Decode */

	/* check that instr is 32-bit length */
	if ((instr & 0x03) != 0x03) {
		verbose_printf("invalid\n");
		fprintf(stderr, "Not 32-bit instruction!\n");
		exit(EXIT_FAILURE);
	} else if ((instr & 0x1C) == 0x1C) { /* 1 1100 */
		verbose_printf("invalid\n");
		fprintf(stderr, "Not 32-bit instruction (2)!\n");
		exit(EXIT_FAILURE);
	}

	switch(get_op(instr)) {
	case OP_LOAD:
		exec_op_load(instr);
		break;
	case OP_IMM:
		exec_op_imm(instr);
		break;
	case OP_AUIPC:
		exec_op_auipc(instr);
		break;
	case OP_STORE:
		exec_op_store(instr);
		break;
	case OP:
		exec_op(instr);
		break;
	case OP_LUI:
		exec_op_lui(instr);
		break;
	case OP_BRANCH:
		exec_op_branch(instr);
		break;
	case OP_JALR:
		exec_op_jalr(instr);
		break;
	case OP_JAL:
		exec_op_jal(instr);
		break;
	case OP_SYSTEM:
		exec_op_system(instr);
		break;
	default:
		trap_invalid_instr();
		//fprintf(stderr, "Invalid instruction!\n");
		break;
	}

	M.regs[0] = 0;

	verbose_printf("\n");

	M.pc += 4;
}

static void print_regs(void) {
	for (int i = 0; i < 32; i += 4) {
		printf("%s=%.8x\t%s=%.8x\t%s=%.8x\t%s=%.8x\n",
				regname[i], M.regs[i], regname[i+1], M.regs[i + 1],
				regname[i+2], M.regs[i + 2], regname[i+3], M.regs[i + 3]);
	}
}


static void parse_args(int argc, char *argv[]) {
	int opt;

	while ((opt = getopt(argc, argv, "vspu")) != -1) {
		switch(opt) {
		case 'v':
			args.verbose = true;
			break;
		case 's':
			args.single_step = true;
			break;
		case 'p':
			args.print_regs = true;
			break;
		case 'u':
			args.enable_uart = true;
			break;
		default: /* '?' */
			goto fail;
		}
	}

	if (optind >= argc)
		goto fail;
	else
		args.bin_file = argv[optind];

	return;

fail:
	fprintf(stderr, "Usage: %s [-vspu] binImage\n", argv[0]);
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
	parse_args(argc, argv);
	mem_init();
	uart_init();
	mem_rom_load_flatbin(0, args.bin_file);

	/* initialize machine state */
	memset(&M, 0, sizeof M);
	M.pc = 0;

	printf("Loaded file into memory: %s\n", args.bin_file);
	printf("Press any key to begin execution...\n");
	getchar();

	for(;;) {
		if (args.enable_uart)
			uart_update_state();
		run_machine_cycle();
		if (args.print_regs) {
			print_regs();
			printf("\n");
		}
		usleep(10);
		if (args.single_step)
			getchar();
	}

	return 0;
}
