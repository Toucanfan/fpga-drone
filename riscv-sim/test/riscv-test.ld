/*OUTPUT_FORMAT("binary");*/
OUTPUT_FORMAT("elf32-littleriscv");
TARGET("elf32-littleriscv");
OUTPUT_ARCH(riscv:rv32);


SECTIONS {
	.rom.text : {
		test/vectors.o(.rom.text)
		*(.rom.text)
	}

	.text : AT(LOADADDR(.rom.text) + SIZEOF(.rom.text) ) {
		*(.text)
	}
	.data : {
		*(.data)
	}
	.bss : {
		*(.bss)
	}
}
