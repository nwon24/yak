/*
 * main.c
 * Main kernel file.
 */

#include <generic/multiboot.h>

#include <asm/interrupts.h>
#include <asm/paging.h>

#include <kernel/debug.h>

#include <drivers/init.h>
#include <drivers/fbcon.h>

#include <mm/mm.h>
#include <mm/vm.h>

/* Internal version of multiboot struct. */
multiboot_info_t __mb_info;

/*
 * Why isn't this called main()?
 * Because at -O2 GCC puts this in .text.startup, which messes with the paging.
 * Since the .text, .rodata, .data and .bss sections are mapped with different
 * permissions, it is vital the mapping is correct and symbols are in the correct
 * sections.
 */
void
kernel_main(multiboot_info_t *mb_info, uint32_t mb_magic)
{
	uint32_t fb;
	int i;

	__mb_info = *mb_info;
	interrupts_init();
	run_critical_driver_init_functions();
	mm_init();
	vm_init();
	run_driver_init_functions();
	printk("Hello, world!\r\n");
	if (mb_magic != MULTIBOOT_BOOTLOADER_MAGIC) {
		printk("Invalid multiboot magic number %x\r\n", mb_magic);
		panic("");
	}
	fbcon_puts("Hello", 5);
	while (1);
}
