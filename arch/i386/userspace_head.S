/*
 * userspace_head.S
 * First code that is run is userspace.
 * It is put in its own section in the kernel image so that it is easy to map.
 */
	.section .user_head, "a"
	.global	first_user_code
first_user_code:
	jmp	.
