/*
 * interrupts.S
 * Has the IRQs for all interrupts.
 */

#include <asm/8259_pic.h>

#include <drivers/driver.h>	

	.section .text
	.global enable_intr
enable_intr:
	sti
	ret

	.global disable_intr
disable_intr:
	cli
	ret

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

/*
 * Start of actual IRQ handlers.
 */
	.global	com_2_4_irq, com_1_3_irq
com_2_4_irq:
com_1_3_irq:
	pusha
	pushl	%ds
	call	acknowledge_int
	pushl	$_DRIVERS_UART8250_DRIVER
	call	driver_irq
	addl	$4, %esp
	popl	%ds
	popa
	iret
