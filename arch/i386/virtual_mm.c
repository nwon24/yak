/*
 * virtual_mm.c
 * Maps physical addresses to virtual addresses.
 */

#include <stddef.h>

#include <asm/paging.h>

#include <kernel/debug.h>

#include <mm/mm.h>
#include <mm/mmap.h>

#define VIRT_ADDR_TMP_PAGE	0xC03FF000

extern uint32_t init_page_directory[];
extern uint32_t init_page_table[];

static uint32_t *current_pg_table = NULL;


static inline void
tmp_map_page(uint32_t *page)
{
	uint32_t virt;

	virt = VIRT_ADDR_TMP_PAGE;
	init_page_table[(virt >> VIRT_ADDR_PG_TAB_SHIFT) & VIRT_ADDR_PG_TAB_MASK] = (uint32_t)page | PAGE_PRESENT | PAGE_WRITABLE;
}

/*
 * Map a page into the virtual kernel address space (>3 GIB)
 * The page should be the physical address of page returned from page_frame_alloc()
 */
uint32_t
kernel_virt_map_page(uint32_t page)
{
	uint32_t *p;

	if (current_pg_table == NULL)
		current_pg_table = init_page_table;
	p = current_pg_table;
	while ((*p & PAGE_PRESENT) && p < current_pg_table + (PAGE_SIZE / sizeof(*p)))
		p++;
	if (p >= current_pg_table + (PAGE_SIZE / sizeof(*p))) {
		uint32_t *tmp;

		/* New page table needed */
		p = (uint32_t *)page_frame_alloc();
		if ((uint32_t)p == NO_FREE_PAGE)
			panic("Out of physical memory");
		tmp_map_page(p);
		tmp = (uint32_t *)(char *)init_page_directory + (KERNEL_VIRT_BASE >> VIRT_ADDR_PG_DIR_SHIFT);
		while ((*tmp & PAGE_PRESENT) && tmp < init_page_directory + (PAGE_SIZE / sizeof(*p) - 1))
			tmp++;
		if (tmp >= init_page_directory + (PAGE_SIZE / sizeof(*p) - 1))
			panic("Out of space in kernel page directory");
		*tmp = (uint32_t)p | PAGE_PRESENT | PAGE_WRITABLE;
		p = (uint32_t *)VIRT_ADDR_TMP_PAGE;
		current_pg_table = p;
	}
	*p = (uint32_t)page | PAGE_WRITABLE | PAGE_PRESENT;
	return KERNEL_VIRT_BASE | (p - current_pg_table) << VIRT_ADDR_PG_TAB_SHIFT;
}
