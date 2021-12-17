/*
 * virtual_mm.c
 * Maps physical addresses to virtual addresses.
 */

#include <stddef.h>

#include <asm/paging.h>

#include <kernel/debug.h>

#include <mm/mm.h>
#include <mm/mmap.h>

/* Virtual address of temporary page table */
#define VIRT_ADDR_TMP_PAGE	0xC03FF000

extern uint32_t init_page_directory[];
extern uint32_t init_page_table[];

static uint32_t *current_pg_table = NULL;

static int virt_map_entry = 1023;

static inline void
tmp_map_page(uint32_t page)
{
	uint32_t virt;

	virt = VIRT_ADDR_TMP_PAGE;
	init_page_table[(virt >> VIRT_ADDR_PG_TAB_SHIFT) & VIRT_ADDR_PG_TAB_MASK] = page | PAGE_PRESENT | PAGE_WRITABLE;
}

static inline uint32_t
get_tmp_page(void)
{
	return init_page_table[(VIRT_ADDR_TMP_PAGE >> VIRT_ADDR_PG_TAB_SHIFT) & VIRT_ADDR_PG_TAB_MASK] & ~VIRT_ADDR_FRAME_MASK;
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
		tlb_flush(VIRT_ADDR_TMP_PAGE);
		tmp_map_page((uint32_t)p);
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

/*
 * A special function called to map a physical address somewhere.
 * It is used when some driver has some data address in physical memory
 * that it has to access (i.e., ACPI tables).
 * 4 MiB is maped at a type for convienience, starting from the upper limit of virtual memory.
 * Each successive call to virt_map_phys moves down the kernel page directory.
 * This makes it as unlikely as possible that this will coincided with the expanding
 * kernel heap, which begins right after the kernel (around 3 GiB).
 * TODO: Write a function to free the virtual memory mapped here.
 * /
 */
uint32_t
virt_map_phys(uint32_t phys)
{
	uint32_t old, pg_table, *p, i, ret;

	old = get_tmp_page();
	if ((pg_table = page_frame_alloc()) == NO_FREE_PAGE)
		return 0;
	tmp_map_page(pg_table);
	tlb_flush(VIRT_ADDR_TMP_PAGE);
	p = (uint32_t *)VIRT_ADDR_TMP_PAGE;
	for (i = phys ; p < (uint32_t *)VIRT_ADDR_TMP_PAGE + (PAGE_SIZE / sizeof(*p)); p++, i += PAGE_SIZE)
		*p = i | PAGE_PRESENT | PAGE_WRITABLE;
	tmp_map_page(old);
	tlb_flush(VIRT_ADDR_TMP_PAGE);
	init_page_directory[virt_map_entry] = pg_table | PAGE_PRESENT | PAGE_WRITABLE;
	ret = virt_map_entry << VIRT_ADDR_PG_DIR_SHIFT;
	for (i = 1023; i; i --) {
		if (!(init_page_directory[i] & PAGE_PRESENT)) {
			virt_map_entry = i;
			break;
		}
	}
	return ret;
}

void
virt_unmap_virt(uint32_t virt)
{
	int entry;
	uint32_t pg_table;

	entry = virt >> VIRT_ADDR_PG_DIR_SHIFT;
	pg_table = init_page_directory[entry] & 0xFFFFF000;
	page_frame_free(pg_table);
	init_page_directory[entry] = 0;
	tlb_flush(virt);
	if (entry > virt_map_entry)
		virt_map_entry = entry;
}
