/*
 * asm_aux.S
 * Auxiliary assembly routines if inline assembly is not to taste.
*/
#include <kernel/config.h>

	.section .text
	/* Some stuff if we don't want to use the inline assembly equivalents. */
#ifndef CONFIG_USE_INLINE_ASM
#include <asm/segment.h>
#include <asm/idt.h>

	.global enable_intr
enable_intr:
	sti
	ret

	.global disable_intr
disable_intr:
	movl	current_cpu_state, %eax
	pushf
	cli
	popl	KERNEL_EFLAGS_OFF(%eax)
	ret

	.global	restore_eflags
restore_eflags:
	movl	current_cpu_state, %.eax
	pushl	KERNEL_EFLAGS_OFF(%eax)
	popf
	ret

	.global	intr_enabled
intr_enabled:
	pushf
	popl	%eax
	andl	$0x200, %eax
	jz	1f
	movl	$1, %eax
1:	ret

	.global load_idt
load_idt:
	movl	4(%esp), %eax
	lidtl	(%eax)
	ret

	/* Shouldn't reall be in this file, but... */
	.global tlb_flush
tlb_flush:
#ifdef CONFIG_X86_ISA_I686
	movl	4(%esp), %eax
	invlpg	(%eax)
	ret
#else
	movl	%cr3, %eax
	movl	%eax, %cr3
	ret
#endif /* CONFIG_X86_ISA_I686 */

	.global	load_cr3
load_cr3:
	movl	4(%esp), %eax
	movl	%eax, %cr3
	ret

	.global	i386_move_to_user
i386_move_to_user:
	movl	$USER_DS_SELECTOR, %eax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs
	movl	%esp, %eax
	pushl	$USER_DS_SELECTOR
	pushl	%eax
	pushfl
	pushl	$USER_CS_SELECTOR
	pushl	$1f
	iret
1:	ret

	.global	get_faulting_address
get_faulting_addr:
	movl	%cr2, %eax
	ret

#endif /* CONFIG_USE_INLINE_ASM */
