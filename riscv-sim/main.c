#include <fcntl.h>
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

static uint32_t get_op(const uint32_t instr) {
	return (instr & 0x1F);
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

static uint32_t get_bit31(const uint32_t instr) {
	return (instr >> 31) & 0x01; /* 1 */
	return 0;
}


static void exec_op_load(uint32_t instr) {
	printf("OP_LOAD\n");
}

static void exec_op_imm(uint32_t instr) {
	printf("OP_IMM\n");
}

static void exec_op_auipc(uint32_t instr) {
	printf("OP_AUIPC\n");
}

static void exec_op_store(uint32_t instr) {
	printf("OP_STORE\n");
}

static void exec_op(uint32_t instr) {
	printf("OP\n");
}

static void exec_op_lui(uint32_t instr) {
	printf("OP_LUI\n");
}

static void exec_op_branch(uint32_t instr) {
	printf("OP_BRANCH\n");
}

static void exec_op_jalr(uint32_t instr) {
	printf("OP_JALR\n");
}

static void exec_op_jal(uint32_t instr) {
	printf("OP_JAL\n");
}

static void exec_op_system(uint32_t instr) {
	printf("OP_SYSTEM\n");
}

static void run_machine_cycle(void) {
	/* Fetch */
	uint32_t instr = M.mem[M.pc];
	printf("%.8x: ", M.pc);
	
	/* Decode */

	/* check that instr is 32-bit length */
	if (instr & 0x03 != 0x03) {
		printf("invalid\n");
		fprintf(stderr, "Not 32-bit instruction!\n");
		exit(EXIT_FAILURE);
	} else if (instr & 0x1C == 0x1C) {
		printf("invalid\n");
		fprintf(stderr, "Not 32-bit instruction!\n");
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
		printf("invalid");
		fprintf(stderr, "Invalid instruction!\n");
		break;
	}

	printf("\n");

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
