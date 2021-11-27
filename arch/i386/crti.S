/*
 * crti.S
 * Kernel version of crti.S, to invoke global constructors.
 */
	.section .init
	.global	_init
	.type	_init, @function
_init:
	pushl	%ebp
	movl	%esp, %ebp
	/* GCC will nicely put the contents of crtbegin.o's .init section here. */

	.section .fini
	.global	_fini
	.type	_fini, @function
_fini:
	pushl	%ebp
	movl	%esp, %ebp
	/* GCC will nicely put the contents of crtbegin.o's .fini section here. */
