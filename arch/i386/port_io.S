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
	outb	%al, %dx
	ret

port_inb:
	xorl	%eax, %eax
	movl	4(%esp), %edx
	inb	%dx, %al
	ret

	.globl	port_outw, port_inw
port_outw:
	movl	4(%esp), %eax
	movl	8(%esp), %edx
	outw	%ax, %dx
	ret

port_inw:
	xorl	%eax, %eax
	movl	4(%esp), %edx
	inw	%dx, %ax
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
