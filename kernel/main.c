/*
 * main.c
 * Main kernel file.
 */

#include <asm/interrupts.h>

void
main(void)
{
	interrupts_init();
	while (1);
}
