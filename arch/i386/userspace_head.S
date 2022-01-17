/*
 * userspace_head.S
 * First code that is run is userspace.
 * It is put in its own section in the kernel image so that it is easy to map.
 */
#include <generic/unistd.h>

	.section .user_head, "a"
	.global	first_user_code
first_user_code:
	movl	$__NR_fork, %eax
	int	$0x80
	test	%eax, %eax
	jz	2f
1:	jmp	.
2:	movl	$__NR_open, %eax
	movl	$file, %ebx
	movl	$0, %ecx
	movl	$0, %edx
	int	$0x80
3:	jmp	.

file:	.asciz	"/dev/tty"
