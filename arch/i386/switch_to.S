/*
 * switch_to.S
 * Context switching code.
 */

	.section .text
	.global asm_switch_to
asm_switch_to:
	movl	current_cpu_state, %esp
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
	/* Utter magic. */
	iret
