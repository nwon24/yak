/*
 * main.c
 * Main kernel file.
 */

#include <asm/interrupts.h>

#include <kernel/debug.h>
#include <kernel/init.h>

#include <drivers/8250_uart.h>

/*
 * Why isn't this called main()?
 * Because at -O2 GCC puts this in .text.startup, which messes with the paging.
 * Since the .text, .rodata, .data and .bss sections are mapped with different
 * permissions, it is vital the mapping is correct and symbols are in the correct
 * sections.
 */
void
kernel_main(void)
{
	interrupts_init();
	run_init_functions();
	printk("Hello, world!\r\n");
	while (1);
}
