/*
 * asm_aux.S
 * Auxiliary assembly routines if inline assembly is not to taste.
*/
#include <kernel/config.h>

	.section .text
	/* Some stuff if we don't want to use the inline assembly equivalents. */
#ifndef CONFIG_USE_INLINE_ASM
#include <asm/segment.h>

	.global enable_intr
enable_intr:
	sti
	ret

	.global disable_intr
disable_intr:
	cli
	ret

	.global load_idt
load_idt:
	movl	4(%ebp), %eax
	lidt	%eax
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
#endif /* CONFIG_USE_INLINE_ASM */
