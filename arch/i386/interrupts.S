/*
 * interrupts.S
 * Has the IRQs for all interrupts.
 */

#include <asm/cpu_state.h>
#include <asm/8259_pic.h>
#include <asm/segment.h>

#include <generic/errno.h>

#include <drivers/driver.h>

	.macro save_regs
	pushl	%ds
	pushl	%es
	pushl	%fs
	pushl	%gs
	pushl	%eax
	pushl	%ebx
	pushl	%ecx
	pushl	%edx
	pushl	%esi
	pushl	%edi
	pushl	%ebp
	.endm

	.macro restore_regs
	popl	%ebp
	popl	%edi
	popl	%esi
	popl	%edx
	popl	%ecx
	popl	%ebx
	popl	%eax
	popl	%gs
	popl	%fs
	popl	%es
	popl	%ds
	addl	$4, %esp
	.endm

	.macro save_state
	pushl	%esp
	call	cpu_state_save
	addl	$4, %esp
	.endm

	.macro restore_kernel_seg
	pushl	%eax
	movl	$KERNEL_DS_SELECTOR, %eax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs
	popl	%eax
	.endm

	.macro exception_handler_no_error handler name
	.global	\name
\name\():
	pushl	$0
	save_regs
	restore_kernel_seg
	save_state
	call	\handler
	restore_regs
	iret
	.endm

	.macro exception_handler_error handler name
	.global	\name
\name\():
	save_regs
	restore_kernel_seg
	save_state
	call	\handler
	restore_regs
	iret
	.endm

	.section .text

	.global	ignore_interrupt
ignore_interrupt:
	pushl	$0
	save_regs
	restore_kernel_seg
	save_state
	calll	acknowledge_int
	jmp	__restart

acknowledge_int:
	movb	$PIC_EOI, %al
	outb	%al, $PIC1_CMD
	calll	pic_read_isr
	cmpw	$PIC2_OFFSET, %ax
	jl	1f
	movb	$PIC_EOI, %al
	outb	%al, $PIC2_CMD
1:	ret

/*
 * Start of actual IRQ handlers.
 */
	.global irq0_handler
irq0_handler:
	pushl	$0
	save_regs
	restore_kernel_seg
	save_state
	call	acknowledge_int
	pushl	$_DRIVERS_TIMER_DRIVER
	call	driver_irq
	addl	$4, %esp
	jmp	__restart

	.global	irq3_handler, irq4_handler
irq3_handler:
irq4_handler:
	pushl	$0
	save_regs
	restore_kernel_seg
	save_state
	call	acknowledge_int
	pushl	$_DRIVERS_UART8250_DRIVER
	call	driver_irq
	addl	$4, %esp
	jmp	__restart

	.global cpu_stop
cpu_stop:
	cli
	hlt
	jmp	cpu_stop

	/* EXCEPTIONS */
exception_handler_no_error do_div_by_zero div_by_zero
exception_handler_no_error do_debug debug
exception_handler_no_error do_nmi nmi
exception_handler_no_error do_breakpoint breakpoint
exception_handler_no_error do_overflow overflow
exception_handler_no_error do_bound_range_exceeded bound_range_exceeded
exception_handler_no_error do_invalid_opcode invalid_opcode
exception_handler_no_error do_dev_not_available dev_not_available
exception_handler_error do_double_fault double_fault
exception_handler_error do_invalid_tss invalid_tss
exception_handler_error do_segment_not_present segment_not_present
exception_handler_error do_stack_segment_fault stack_segment_fault
exception_handler_error do_gp_fault gp_fault
exception_handler_error do_page_fault page_fault
exception_handler_no_error do_x87_floating_point x87_floating_point
exception_handler_error do_alignment_check alignment_check
exception_handler_no_error do_machine_check machine_check
exception_handler_no_error do_simd_floating_point simd_floating_point
exception_handler_no_error do_virtualization virtualization
exception_handler_error do_control_protection control_protection
exception_handler_no_error do_reserved reserved

	/* Syscall handler (IRQ 0x80) */
	.global syscall
syscall:
	/* Save %eax immediately - contains syscall number */
	pushl	%eax
	movl	%eax, syscall_nr
	popl	%eax
	pushl	$0
	save_regs
	restore_kernel_seg
	save_state
	pushl	syscall_nr
	pushl	current_cpu_state
	call	handle_syscall
	addl	$8, %esp
	jmp	__restart
	
	/* Two underscores because this is extra special */
	.global	__restart
__restart:
	restore_regs
	iret

	.section .data
syscall_nr:
	.long	0

iret_frame:
	.long	0
