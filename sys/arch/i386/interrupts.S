/*
 * interrupts.S
 * Has the IRQs for all interrupts.
 */

#include <asm/cpu_state.h>
#include <asm/pic_8259.h>
#include <asm/segment.h>

#include <generic/errno.h>

#include <drivers/driver.h>

#include <kernel/proc.h>

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
	jmp	restart

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
	pushl	$DRIVERS_TIMER_DRIVER
	call	driver_irq
	addl	$4, %esp
	jmp	restart

	.global	irq1_handler
irq1_handler:
	pushl	$0
	save_regs
	restore_kernel_seg
	save_state
	call	acknowledge_int
	pushl	$DRIVERS_KEYBOARD_DRIVER
	call	driver_irq
	addl	$4, %esp
	jmp	restart

	.global	irq3_handler, irq4_handler
irq3_handler:
irq4_handler:
	pushl	$0
	save_regs
	restore_kernel_seg
	save_state
	call	acknowledge_int
	pushl	$DRIVERS_UART8250_DRIVER
	call	driver_irq
	addl	$4, %esp
	jmp	restart

	.global	irq12_handler
irq12_handler:
	pushl	$0
	save_regs
	restore_kernel_seg
	save_state
	call	acknowledge_int
	pushl	$DRIVERS_MOUSE_DRIVER
	call	driver_irq
	addl	$4, %esp
	jmp	restart

	.global	irq14_handler, irq15_handler
irq14_handler:
irq15_handler:
	pushl	$0
	save_regs
	restore_kernel_seg
	save_state
	call	acknowledge_int
	pushl	$DRIVERS_DISK_DRIVER
	call	driver_irq
	addl	$4, %esp
	jmp	restart

	.global cpu_stop
cpu_stop:
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
	movl	current_cpu_state, %eax
	movl	%esp, IRET_FRAME_OFF(%eax)
	pushl	syscall_nr
	pushl	current_cpu_state
	call	handle_syscall
	addl	$8, %esp
	/*
	 * Signal recognition is only done during swtich
	 * from kernel to user space.
	 */
	movl	IRET_FRAME_CS(%esp), %ebx
	pushl	%eax
	testl	$3, %ebx
	jz	1f
	subl	$4, %esp
	pushl	%esp
	pushl	current_process
	/*
	 * Not actually a signal handler, but the function
	 * that gets the signal handler.
	 */
	call	signal_handler
	movl	4(%esp), %ecx
	addl	$12, %esp
	testl	%eax, %eax
	jz	1f
	xchgl	%eax, IRET_FRAME_EIP(%esp)
	subl	$8, IRET_FRAME_ESP(%esp)
	movl	IRET_FRAME_ESP(%esp), %edx
	movl	current_process, %ebx
	movl	SIG_RESTORER(%ebx), %eax
	movl	%eax, (%edx)
	movl	%ecx, 4(%edx)
	movl	%edx, IRET_FRAME_ESP(%esp)
1:	popl	%eax
	popl	%ebp
	popl	%edi
	popl	%esi
	popl	%edx
	popl	%ecx
	popl	%ebx
	/* Skip over %eax as that is the return value of the syscall */
	addl	$4, %esp
	popl	%gs
	popl	%fs
	popl	%es
	popl	%ds
	addl	$4, %esp
	iret

	.global	restart
restart:
	restore_regs
	iret

	.section .data
syscall_nr:
	.long	0

iret_frame:
	.long	0
