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
	pushl	$0
	pushl	$0
	movl	%esp, %ebp

	pushl	12(%ebp)
	pushl	8(%ebp)
	call	__libc_init
	call	_init
	call	main

	pushl	%eax
	call	_fini
	call	__libc_fini
	popl	%ebx
	movl	$SYS_exit, %eax
	int	$SYSCALL_IRQ
