#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>

#define MEMBIT 24

struct machine {
	int pc;
	char *mem;
	uint32_t regs[32];
} M;

enum {
	E_INVAL_INSTR = 0,
	E_SOME_CRAP,
}

static void machine_loop(void) {
	/* Fetch */
	uint32_t instr = M[pc];
	
	/* Decode */
	if (instr & 0x03 != 0x03)

	switch(instr & 0x02) {
	case :


int main(int argc, char *argv[]) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s binImage\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	int fd = open(argv[1], O_RDONLY);

	struct stat st;
	if (fstat(fd, &st) < 0) {
		perror("fstat");
		exit(EXIT_FAILURE);
	}
	int programSize = st.st_size;

	char *program = mmap(NULL, programSize, PROT_READ|PROT_WRITE, 
			MAP_PRIVATE, fd, 0);
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

	machine_loop();

	return 0;
}


