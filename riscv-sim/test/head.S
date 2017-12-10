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
#call memtest
	call copy_data /* copy .data to RAM */
	call main

	/* we should never return here */
	j panic

memtest:
	addi sp,sp,-32

	addi s1,zero,1
	sw s1,28(sp)

	addi s1,s1,1
	sw s1,24(sp)

	addi s1,s1,1
	sw s1,20(sp)

	lw s2,28(sp)
	lw s3,24(sp)
	lw s4,20(sp)

	addi sp,sp,32
