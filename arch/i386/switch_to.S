/*
 * switch_to.S
 * Context switching code.
 */
#include <asm/cpu_state.h>

/*
 * WARNING: Huge comment.
 * This might be a bit confusing as it looks like a normal function with a 'ret' at the end.
 * What does function is actually doing is restoring the 'iret' frame of a process.
 * At each interrupt (see interrupts.S), the value of the stack pointer after %ebp has been pushed is saved
 * in the 'iret_frame' member of the current cpu state struct. This function simply copies the values
 * from the current cpu state struct to the saved iret_frame address, thus changing the iret frame when this
 * function returns to the interrupt handlers in interrupts.S. This function needs to check if the state we
 * are restoring is one that came from userspace. If so, it also restores the %ss and %esp fields of the 
 * iret frame. Each interrupt handler then jumps to '__restart' which restores all the registers from the iret frame,
 * before executing the magic iret instruction.
 */
	.section .text
	.global asm_switch_to
asm_switch_to:
	cld
	pushl	%esi
	pushl	%edi
	movl	current_cpu_state, %esi
	movl	IRET_FRAME_OFF(%esi), %edi
	movl	CS_OFF(%esi), %eax
	andl	$3, %eax
	jz	1f
	movl	$17*4, %ecx
	rep
	movsl
	jmp	2f
1:	movl	$15*4, %ecx
	rep
	movsl
2:	popl	%edi
	popl	%esi
	ret
