/*
 * syscall.S
 */
#include <syscall.h>

	.section .text
	.globl syscall
syscall:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx

	movl	8(%ebp), %eax
	movl	12(%ebp), %ebx
	movl	16(%ebp), %ecx
	movl	20(%ebp), %edx
	movl	24(%ebp), %esi
	movl	28(%ebp), %edi
	int	$SYSCALL_IRQ

	cmpl	$0, %eax
	je	1f
	movl	%eax, errno
	movl	$-1, %eax
1:	popl	%ebx
	popl	%esi
	popl	%edi
	movl	%ebp, %esp
	popl	%ebp
	ret
