	.section .init
	/* GCC will put stuff here */
	popl	%ebp
	ret

	.section .fini
	/* GCC will put stuff here */
	popl	%ebp
	ret
