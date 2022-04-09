/*
 * userspace_head.S
 * First code that is run is userspace.
 * It is put in its own section in the kernel image so that it is easy to map.
 */
#include <generic/unistd.h>
#include <generic/fcntl.h>
#include <generic/stat.h>

	.section .user_head, "a"
	.global	first_user_code
first_user_code:
	movl	$__NR_fork, %eax
	int	$0x80
	cmpl	$0, %eax	/* Error? */
	jl	1f		/* Yes, report it */
	je	2f		/* No, child does exec */
	jmp	.		/* Parent does nothing, becomes idle process */
2:	movl	$__NR_execve, %eax
	movl	$execf, %ebx
	movl	$argv, %ecx
	movl	$envp, %edx
	int	$0x80
	/* Unreachable unless error in execve */
	movl	$__NR_write, %eax
	movl	$1, %ebx
	movl	$execve_msg, %ecx
	movl	$execve_msg_len, %edx
	int	$0x80
	jmp	.
1:	movl	$__NR_write, %eax
	movl	$1, %ebx
	movl	$fork_msg, %ecx
	movl	fork_msg_len, %edx
	int	$0x80
	jmp	.

fork_msg:
	.asciz	"fork() failed in 'first_user_code'"
fork_msg_len:
	.long	.-fork_msg
execve_msg:
	.asciz	"execve() failed in 'first_user_code'"
execve_msg_len:
	.long	.-execve_msg
execf:	.asciz	"/usr/bin/init"
argv:	.long	execf
	.long	0
envp:	.long	0
