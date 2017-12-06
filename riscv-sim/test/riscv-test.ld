/*OUTPUT_FORMAT("binary");*/
OUTPUT_FORMAT("elf32-littleriscv");
TARGET("elf32-littleriscv");
OUTPUT_ARCH(riscv:rv32);


SECTIONS {
	. = 0x00000000;

	.text : {
		*(.early.text)
		*(.text)
	}

	.rodata : {
		*(.srodata)
		*(.rodata)
	}

	lma_sdata = .;

	. = 0x00010000;

	vma_sdata = .;
	.data : AT(lma_sdata) {
		*(.sdata)
		*(.data)
	}
	vma_edata = .;
	data_size = SIZEOF(.data);

	.bss : {
		*(.sbss)
		*(.bss)
	}
}
