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
	movl	$__NR_open, %eax
	movl	$file2, %ebx
	movl	$0, %ecx
	movl	$0, %edx
	int	$0x80
	cmpl	$5, %eax
	jne	1f
	cli
	hlt
1:	movl	%eax, %esi
	movl	$__NR_read, %eax
	movl	%esi, %ebx
	movl	$buf, %ecx
	movl	$5, %edx
	int	$0x80
	movl	$__NR_write, %eax
	movl	$0, %ebx
	movl	$buf, %ecx
	movl	$5, %edx
	int	$0x80
3:	jmp	.

file:	.asciz	"/dev/tty"
file2:	.asciz 	"/boot/grub/grub.cfg"
msg:	.asciz	"Hello, world!"
buf:	.space	5
