/*
 * vm_init.c
 * Initialise virtual memory manager.
 * Shouldn't really be in its own file...
 */

#include <asm/paging.h>

#include <kernel/debug.h>

#include <mm/mm.h>
#include <mm/vm.h>

static uint32_t kernel_heap_start;

void
vm_init(void)
{
	int i;

	/*
	 * We coun't on there being at least 4 MiB free memory.
	 * Even QEMU has 128 MiB free memory by default.
	 */
	for (i = 0; i < HEAP_SIZE / PAGE_SIZE; i++) {
		if (!i)
			kernel_heap_start = kernel_virt_put_page();
		else
			kernel_virt_put_page();
	}
	printk("Kernel heap begins at %x\r\n", kernel_heap_start);
	kvmalloc_init(kernel_heap_start, HEAP_SIZE);
}
