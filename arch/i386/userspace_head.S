/*
 * userspace_head.S
 * First code that is run is userspace.
 * It is put in its own section in the kernel image so that it is easy to map.
 */
#include <generic/unistd.h>
#include <generic/fcntl.h>

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
	movl	$O_RDWR, %ecx
	movl	$0, %edx
	int	$0x80
	movl	$__NR_open, %eax
	movl	$file2, %ebx
	movl	$O_RDWR | O_CREAT, %ecx
	movl	$0100664, %edx
	int	$0x80
	movl	%eax, fd
	movl	$__NR_write, %eax
	movl	fd, %ebx
	movl	$buf2, %ecx
	movl	$5, %edx
	int	$0x80
	movl	$__NR_lseek, %eax
	movl	fd, %ebx
	movl	$0, %ecx
	movl	$SEEK_SET, %edx
	int	$0x80
	movl	$__NR_read, %eax
	movl	fd, %ebx
	movl	$buf, %ecx
	movl	$5, %edx
	int	$0x80
	movl	$__NR_write, %eax
	movl	$0, %ebx
	movl	$buf, %ecx
	movl	$5, %edx
	int	$0x80
	movl	$__NR_link, %eax
	movl	$file2, %ebx
	movl	$file3, %ecx
	int	$0x80
	movl	$__NR_unlink, %eax
	movl	$file2, %ebx
	int	$0x80
	movl	$__NR_unlink, %eax
	movl	$file3, %ebx
	int	$0x80
	movl	$__NR_sync, %eax
	int	$0x80
3:	jmp	.

file:	.asciz	"/dev/tty"
file2:	.asciz 	"/test"
file3:	.asciz	"/test2"
msg:	.asciz	"Hello, world!"
fd:	.long	0
buf:	.space	5
buf2:	.asciz	"and ?"
