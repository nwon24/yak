/*
 * crt0.S
 * The stuff that runs before 'main'.
 */
#include <unistd.h>
#include <syscall.h>

	.section .text
	.globl _start
	.type	_start, @function
_start:
	xorl	%ebp, %ebp
	pushl	%ebp
	pushl	%ebp
	movl	%esp, %ebp

	pushl	12(%ebp)
	pushl	8(%ebp)
	call	__libc_init
	call	_init
	call	main

	movl	%eax, %ebx
	movl	$SYS_exit, %eax
	int	$SYSCALL_IRQ
