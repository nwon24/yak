/*
 * interrupts.S
 * Has the IRQs for all interrupts.
 */

#include <asm/8259_pic.h>

	.section .text
	.global	ignore_interrupt
ignore_interrupt:
	pusha
	pushl	%ds
	/* Do not need to move kernel data segment into %ds for now */
	calll	acknowledge_int
	popl	%ds
	popa
	iretl

acknowledge_int:
	movb	$PIC_EOI, %al
	outb	%al, $PIC1_CMD
	calll	pic_read_isr
	cmpw	$PIC2_OFFSET, %ax
	jl	1f
	movb	$PIC_EOI, %al
	outb	%al, $PIC1_CMD
1:	ret
