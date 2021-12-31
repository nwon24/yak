/*
 * port_io.S
 * Port IO functions.
 * Use this if inline assembly is not to taste.
 */

	.section .text
	.globl	port_outb, port_inb
port_outb:
	movl	4(%esp), %eax
	movl	8(%esp), %edx
	outb	%al, %edx
	ret

port_inb:
	xorl	%eax, %eax
	movl	4(%esp), %edx
	inb	%edx, %al
	ret

	.globl	port_outw, port_inw
port_outw:
	movl	4(%esp), %eax
	movl	8(%esp), %edx
	outw	%ax, %edx
	ret

port_inw:
	xorl	%eax, %eax
	movl	4(%esp), %edx
	inw	%edx, %ax
	ret

	.globl	port_outl, port_inl
port_outl:
	movl	4(%esp), %eax
	movl	8(%esp), %edx
	outl	%eax, %edx
	ret

port_inl:
	xorl	%eax, %eax
	movl	4(%esp), %edx
	inl	%edx, %eax
	ret

	.globl	port_rep_insb, port_rep_insw, port_rep_insl
port_rep_insb:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi

	movl	8(%esi), %edx
	movl	12(%esp), %edi
	movl	16(%esp), %ecx
	cld
	rep
	insb

	popl	%edi
	movl	%ebp, %esp
	popl	%ebp
	ret

port_rep_insw:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi

	movl	8(%esi), %edx
	movl	12(%esp), %edi
	movl	16(%esp), %ecx
	cld
	rep
	insw

	popl	%edi
	movl	%ebp, %esp
	popl	%ebp
	ret

port_rep_insl:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi

	movl	8(%esi), %edx
	movl	12(%esp), %edi
	movl	16(%esp), %ecx
	cld
	rep
	insl

	popl	%edi
	movl	%ebp, %esp
	popl	%ebp
	ret

	.globl	io_delay
io_delay:
	jmp	1f
1:	jmp	1f
1:	ret
