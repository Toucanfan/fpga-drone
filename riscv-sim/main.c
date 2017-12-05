#include <fcntl.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>

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

struct machine {
	uint32_t pc;
	char *mem;
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
	return (instr >> 12) & 0x03; /* 11 */
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
	return (imm_12 << 12)|(imm_11 << 11)|(imm_10_5 << 10)|(imm_4_1 << 4);
}

static uint32_t get_u_imm(const uint32_t instr) {
	return instr & 0xFFFFF000; /* 1111 1111 1111 1111 1111 0000 0000 0000 */
}

static uint32_t get_j_imm(const uint32_t instr) {
	uint32_t imm_20 = (instr >> 31) & 0x01; /* 1 */
	uint32_t imm_10_1 = (instr >> 21) & 0x3FF; /* 11 1111 1111 */
	uint32_t imm_11 = (instr >> 20) & 0x01; /* 1 */
	uint32_t imm_19_12 = (instr >> 12) & 0xFF; /* 1111 1111 */
	return (imm_20 << 20)|(imm_19_12 << 19)|(imm_11 << 11)|(imm_10_1 << 10);
}

static void verbose_printf(const char *format, ...) {
	va_list ap;
	va_start(ap, format);

	vprintf(format, ap);
}


static void trap_invalid_instr(void) {
	verbose_printf("invalid ");
}

static void exec_op_load(uint32_t instr) {
	uint32_t rd = get_rd(instr);
	uint32_t rs1 = get_rs1(instr);
	uint32_t offset = sign_extend(get_i_imm(instr), 11);
	uint32_t eff = rs1 + offset;
	switch(get_funct3(instr)) {
	case 0: /* 000 LB */
		M.regs[rd] = sign_extend((uint32_t)(*((uint8_t *)&M.mem[eff])), 7);
		verbose_printf("lb x%d,%d(x%d) ", rd, (int32_t)offset, rs1);
		break;
	
	case 1: /* 001 LH */
		M.regs[rd] = sign_extend((uint32_t)(*((uint16_t *)&M.mem[eff])), 15);
		verbose_printf("lh x%d,%d(x%d) ", rd, (int32_t)offset, rs1);
		break;

	case 2: /* 010 LW */
		M.regs[rd] = *((uint32_t *)&M.mem[eff]);
		verbose_printf("lw x%d,%d(x%d) ", rd, (int32_t)offset, rs1);
		break;

	case 4: /* 100 LBU */
		M.regs[rd] = (uint32_t)(*((uint8_t *)&M.mem[eff]));
		verbose_printf("lbu x%d,%d(x%d) ", rd, (int32_t)offset, rs1);
		break;

	case 5: /* 101 LHU */
		M.regs[rd] = (uint32_t)(*((uint16_t *)&M.mem[eff]));
		verbose_printf("lhu x%d,%d(x%d) ", rd, (int32_t)offset, rs1);
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
		verbose_printf("addi x%d,x%d,%d ", rd, rs1, imm);
		break;
	case 1: /* 001 */
		switch(funct7) {
		case 0x00: /* 000 0000 SLLI */
			M.regs[rd] = M.regs[rs1] << shamt;
			verbose_printf("slli x%d,x%d,%d ", rd, rs1, shamt);
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
		verbose_printf("slti x%d,x%d,%d ", rd, rs1, imm);
		break;
	case 3: /* 011 SLTIU */
		imm = sign_extend(imm, 11);
		if (M.regs[rs1] < imm)
			M.regs[rd] = 1;
		else
			M.regs[rd] = 0;
		verbose_printf("sltiu x%d,x%d,%d ", rd, rs1, imm);
		break;
	case 4: /* 100 XORI */
		imm = sign_extend(imm, 11);
		M.regs[rd] = M.regs[rs1] ^ imm;
		verbose_printf("xori x%d,x%d,%d ", rd, rs1, imm);
		break;
	case 5: /* 101 */
		switch(funct7) {
		case 0x00: /* 000 0000 SRLI */
			M.regs[rd] = M.regs[rs1] >> shamt;
			verbose_printf("srli x%d,x%d,%d ", rd, rs1, shamt);
			break;
		case 0x20: /* 010 0000 SRAI */
			M.regs[rd] = (uint32_t)(((int32_t)M.regs[rs1]) >> shamt);
			verbose_printf("srai x%d,x%d,%d ", rd, rs1, shamt);
			break;
		default:
			trap_invalid_instr();
			break;
		}
		break;
	case 6: /* 110 ORI */
		imm = sign_extend(imm, 11);
		M.regs[rd] = M.regs[rs1] | imm;
		verbose_printf("ori x%d,x%d,%d ", rd, rs1, imm);
		break;
	case 7: /* 111 ANDI */
		imm = sign_extend(imm, 11);
		M.regs[rd] = M.regs[rs1] & imm;
		verbose_printf("andi x%d,x%d,%d ", rd, rs1, imm);
		break;
	default:
		trap_invalid_instr();
		break;
	}
	/* verbose_printf("OP_IMM "); */
}

static void exec_op_auipc(uint32_t instr) {
	verbose_printf("OP_AUIPC ");
}

static void exec_op_store(uint32_t instr) {
	verbose_printf("OP_STORE ");
}

static void exec_op(uint32_t instr) {
	verbose_printf("OP ");
}

static void exec_op_lui(uint32_t instr) {
	verbose_printf("OP_LUI ");
}

static void exec_op_branch(uint32_t instr) {
	verbose_printf("OP_BRANCH ");
}

static void exec_op_jalr(uint32_t instr) {
	verbose_printf("OP_JALR ");
}

static void exec_op_jal(uint32_t instr) {
	verbose_printf("OP_JAL ");
}

static void exec_op_system(uint32_t instr) {
	verbose_printf("OP_SYSTEM ");
}

static void run_machine_cycle(void) {
	/* Fetch */
	uint32_t instr = *((uint32_t *)&M.mem[M.pc]);
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

	verbose_printf("\n");

	M.pc += 4;
}


int main(int argc, char *argv[]) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s binImage\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	int fd = open(argv[1], O_RDONLY);
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

	char *program = mmap(NULL, programSize, PROT_READ, MAP_PRIVATE, fd, 0);
	if (program == MAP_FAILED) {
		perror("mmap");
		exit(EXIT_FAILURE);
	}

	memset(&M, 0, sizeof M);
	M.pc = 0;
	M.mem = malloc(1 << MEMBIT);
	memset(M.mem, 0, (1 << MEMBIT));

	for (int i = 0; i < programSize; ++i) {
		M.mem[i] = program[i];
	}

	for(;;) {
		run_machine_cycle();
	}

	return 0;
}
