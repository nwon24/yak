/*
 * switch_to.S
 * Context switching code.
 */
#include <asm/cpu_state.h>

/*
 * WARNING: Huge comment.
 * This function looks like a little bit of magic. What happens is it gets the address of the last 
 * saved iret frame, and then copies the new CPU context to it. It checks where the %cs register
 * contains a selector for CPL 3 (meaning the interrupt came from userspace), in which case %ss and %esp
 * must be copied also (hence copy 17 double words instead of 15). Then it jumps to '__restart' from interrupts.S
 * that simply pops off the registers just copied before exceuting the 'iret' instruction.
 */
	.section .text
	.global asm_switch_to
asm_switch_to:
	cld
	movl	current_cpu_state, %esi
	movl	IRET_FRAME_OFF(%esi), %edi
	movl	CS_OFF(%esi), %eax
	andl	$3, %eax
	jz	1f
	movl	$17, %ecx
	rep
	movsl
	jmp	2f
1:	movl	$15, %ecx
	rep
	movsl
2:	movl	current_cpu_state, %esi
	movl	IRET_FRAME_OFF(%esi), %esp
	jmp	__restart
