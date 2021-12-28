/*
 * syscall.S
 * Dispatches to a system call.
 */

	.section .text
	.global	handle_syscall
handle_syscall:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx

	movl	$syscall_table, %edi
	movl	12(%ebp), %eax
	shll	$3, %eax
	addl	%eax, %edi
	movl	(%edi), %eax
	/* %ebx = addr, %ecx = nr_args */
	movl	%eax, %ebx
	movl	4(%edi), %eax
	movl	%eax, %ecx

	movl	8(%ebp), %edi
	cmpl	$6, %ecx
	jl	1f
	pushl	(%edi)
1:	cmpl	$5,%ecx
	jl	1f
	pushl	4(%edi)
1:	cmpl	$4,%ecx
	jl	1f
	pushl	8(%edi)
1:	cmpl	$3,%ecx
	jl	1f
	pushl	12(%edi)
1:	cmpl	$2,%ecx
	jl	1f
	pushl	16(%edi)
1:	cmpl	$1,%ecx
	jl	1f
	pushl	20(%edi)
1:	movl	%ecx, %edi		/* Move %ecx to a callee saved register */
	sti
	call	*%ebx
	cli
	shll	$2, %edi
	addl	%edi, %esp

	popl	%ebx
	popl	%esi
	popl	%edi
	movl	%ebp, %esp
	popl	%ebp
	ret
