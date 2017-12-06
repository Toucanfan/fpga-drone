.section ".early.text"

/* Exception vectors */
reset:	j	start
undef:	j	undef
panic:	j	panic
irq:	j	irq

.globl main
.globl copy_data

start:
	li sp, 0x20000
	call copy_data /* copy .data to RAM */
	call main

	/* we should never return here */
	j panic
