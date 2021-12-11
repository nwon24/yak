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
