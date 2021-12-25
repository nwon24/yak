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
1:	movl	$0, %eax
	movl	$0, %ebx
	int	$0x80
	jmp	1b
2:	movl	$0, %eax
	movl	$1, %ebx
	int	$0x80
	jmp	2b
3:	jmp	.
