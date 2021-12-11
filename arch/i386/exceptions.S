/*
 * exceptions.S
 * All exceptions are trapped in this file.
 */
	.section .text
	.global	fatal_exception
fatal_exception:
	cli
	hlt
