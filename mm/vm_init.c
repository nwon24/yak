/*
 * vm_init.c
 * Initialise virtual memory manager.
 */

#include <asm/paging.h>

#include <mm/mm.h>
#include <mm/vm.h>

#define INIT_HEAP_SIZE	0x400000

void
vm_init(void)
{
	int i;

	for (i = 0; i < INIT_HEAP_SIZE / PAGE_SIZE; i++)
		kernel_virt_put_page();
}
