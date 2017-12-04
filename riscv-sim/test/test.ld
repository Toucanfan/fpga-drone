/*OUTPUT_FORMAT("binary");*/
SECTIONS {
	.rom.text : {
		vectors.o(.rom.text)
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
