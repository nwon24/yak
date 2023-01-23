/*
 * virtual_mm.c
 * Contains code to deal with virtual memory.
 */

#include <stddef.h>

#include <asm/cpu_state.h>
#include <asm/paging.h>

#include <kernel/exec.h>
#include <kernel/debug.h>
#include <kernel/proc.h>

#include <generic/string.h>

#include <mm/mm.h>
#include <mm/mmap.h>

/* Virtual address of temporary pages */
#define VIRT_ADDR_TMP_PAGE	0xC03FF000

extern uint32_t init_page_directory[];
extern uint32_t init_page_table[];

static uint32_t *current_pg_table = NULL;

static int virt_map_entry = 1023;

static void free_page_table(uint32_t page_table);
static void pg_table_cow(uint32_t pg_table, int inc_ref);

static void page_set_writable(uintptr_t addr, int copy);

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

/*
 * Only to be used for kernel memory.
 */
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

/*
 * Used to map a section of the virtual address into the given
 * page directory.
 * The address of the page directory is its physical one, such as one
 * returned  from 'page_frame_alloc'.
 * This code is quite unreadable.
 * If 'phys' is 0, then the physical address doesn't matter.
 */
uint32_t
virt_map_chunk(uint32_t start, uint32_t size, uint32_t *page_dir, int flags, uint32_t phys)
{
	uint32_t pg_table, pg_dir, old, tmp;
	int nr_tables, i, j, index;

	old = get_tmp_page();
	/*
	 * Caller has option of not providing a page table, in which case we allocate
	 * it here.
	 */
	if (page_dir == NULL) {
		pg_dir = page_frame_alloc();
		if (pg_dir == NO_FREE_PAGE) {
			tmp_map_page(old);
			tlb_flush(VIRT_ADDR_TMP_PAGE);
			goto error;
		}
	} else {
		pg_dir = (uint32_t)page_dir;
	}
	tmp_map_page(pg_dir);
	tlb_flush(VIRT_ADDR_TMP_PAGE);
	if (page_dir == NULL)
		memmove((void *)VIRT_ADDR_TMP_PAGE, (void *)init_page_directory, PAGE_SIZE);
	/* Figure out the number of page tables needed. Each page table can map up to 4 MiB of memory */
	nr_tables = size / 0x400000 + 1;
	tmp = start & 0xFFFFF000;
	for (j = 0; j < nr_tables; j++) {
		index = tmp >> VIRT_ADDR_PG_DIR_SHIFT;
		tmp_map_page(pg_dir);
		tlb_flush(VIRT_ADDR_TMP_PAGE);
		if (!((pg_table = *(uint32_t *)(VIRT_ADDR_TMP_PAGE + (index << 2) + (j << 2))) & PAGE_PRESENT)) {
			if ((pg_table = page_frame_alloc()) == NO_FREE_PAGE) {
				tmp_map_page(old);
				tlb_flush(VIRT_ADDR_TMP_PAGE);
				goto error;
			}
			tmp_map_page(pg_table);
			tlb_flush(VIRT_ADDR_TMP_PAGE);
			memset((void *)VIRT_ADDR_TMP_PAGE, 0, PAGE_SIZE);
		} else {
			pg_table &= 0xFFFFF000;
			tmp_map_page(pg_table);
			tlb_flush(VIRT_ADDR_TMP_PAGE);
		}
		for (i = ((tmp >> VIRT_ADDR_PG_TAB_SHIFT) & VIRT_ADDR_PG_TAB_MASK) << 2; i < PAGE_SIZE; i += 4, tmp += PAGE_SIZE) {
			uint32_t page_frame;

			if (tmp > start + size)
				break;
			if (phys == 0) {
				page_frame = page_frame_alloc();
				if (page_frame == NO_FREE_PAGE)
					goto error;
				*(uint32_t *)(VIRT_ADDR_TMP_PAGE + i) = page_frame | PAGE_PRESENT | flags;
			} else {
				*(uint32_t *)(VIRT_ADDR_TMP_PAGE + i) = phys | PAGE_PRESENT | flags;
				phys += PAGE_SIZE;
			}
		}
		tmp_map_page(pg_dir);
		tlb_flush(VIRT_ADDR_TMP_PAGE);
		*(uint32_t *)(VIRT_ADDR_TMP_PAGE + (index << 2) + (j << 2)) = pg_table | PAGE_PRESENT | flags;
	}
	tmp_map_page(old);
	tlb_flush(VIRT_ADDR_TMP_PAGE);
	return pg_dir;
error:
	return 0;
}

/*
 * Is actually quite simple.
 * We simply mark the page directory as read only. We only copy the
 * page once it is written to and a page fault appears.
 */
void
copy_address_space(uint32_t from_page_dir, uint32_t to_page_dir)
{
	uint32_t *p, old, new;
	uint32_t tmp_page[PAGE_SIZE / sizeof(uint32_t)];

	/* Addresses passed are physical address so this is a bit of a pain. */
	old = get_tmp_page();
	tmp_map_page(from_page_dir);
	tlb_flush(VIRT_ADDR_TMP_PAGE);
	memmove(tmp_page, (void *)VIRT_ADDR_TMP_PAGE, PAGE_SIZE);
	tmp_map_page(to_page_dir);
	tlb_flush(VIRT_ADDR_TMP_PAGE);
	memmove((void *)VIRT_ADDR_TMP_PAGE, tmp_page, PAGE_SIZE);
	/*
	 * If we are doing the first fork, don't bother with copy on write as it should
	 * only be a single page. And because the first process after doing the fork is idle
	 * and therefore doesn't write to any memory, we don't even have to copy the page itself.
	 */
	if (current_process->pid == 0)
		goto out;
	/*
	 * Mark both page directories for copy on write.
	 * Only need to increase reference count once, so we don't do it on the first 'page_table_cow'
	 */
	for (p = (uint32_t *)VIRT_ADDR_TMP_PAGE; p < (uint32_t *)VIRT_ADDR_TMP_PAGE + PAGE_SIZE / sizeof(uint32_t); p++) {
		if ((*p & PAGE_PRESENT) && (*p & PAGE_USER))
			pg_table_cow(*p, 0);
	}
	tmp_map_page(from_page_dir);
	tlb_flush(VIRT_ADDR_TMP_PAGE);
	for (p = (uint32_t *)VIRT_ADDR_TMP_PAGE; p < (uint32_t *)VIRT_ADDR_TMP_PAGE + PAGE_SIZE / sizeof(uint32_t); p++) {
		if ((*p & PAGE_PRESENT) && (*p & PAGE_USER)) {
			int old_flags;

			new = page_frame_alloc();
			if (new == NO_FREE_PAGE)
				panic("Out of memory");
			copy_page_table((uint32_t *)new, (uint32_t *)(*p & 0xFFFFF000));
			pg_table_cow(new, 1); 
			old_flags = *p & 0xFFF;
			*p = new | old_flags;
			*p &= ~PAGE_WRITABLE;
		}
	}
out:
	tmp_map_page(old);
	tlb_flush(VIRT_ADDR_TMP_PAGE);
}

static void
pg_table_cow(uint32_t pg_table, int inc_ref)
{
	uint32_t old, *p;

	old = get_tmp_page();
	tmp_map_page(pg_table & 0xFFFFF000);
	tlb_flush(VIRT_ADDR_TMP_PAGE);
	for (p = (uint32_t *)VIRT_ADDR_TMP_PAGE; p < (uint32_t *)(VIRT_ADDR_TMP_PAGE + PAGE_SIZE); p++) {
		if (*p & PAGE_PRESENT) {
			*p &= ~PAGE_WRITABLE;
			if (inc_ref)
				page_increase_count(*p & 0xFFFFF000);
		}
	}
	tmp_map_page(old);
	tlb_flush(VIRT_ADDR_TMP_PAGE);
}

/*
 * Check that a pointer provided by userspace is valid.
 */
int
check_user_ptr(void *addr)
{
	uint32_t *pg_dir, *pg_table, tmp, old;
	int ret = 0;

	tmp = (uint32_t)addr;
	old = get_tmp_page();

	pg_dir = (uint32_t *)(current_cpu_state->cr3);
	tmp_map_page((uint32_t)pg_dir);
	tlb_flush(VIRT_ADDR_TMP_PAGE);
	pg_dir = (uint32_t *)VIRT_ADDR_TMP_PAGE;
	if (!(pg_dir[tmp >> VIRT_ADDR_PG_DIR_SHIFT] & PAGE_PRESENT) || !(pg_dir[tmp >> VIRT_ADDR_PG_DIR_SHIFT] & PAGE_USER))
		goto out;
	pg_table = (uint32_t *)(pg_dir[tmp >> VIRT_ADDR_PG_DIR_SHIFT] & 0xFFFFF000);
	tmp >>= VIRT_ADDR_PG_TAB_SHIFT;
	tmp &= VIRT_ADDR_PG_TAB_MASK;
	tmp_map_page((uint32_t)pg_table);
	tlb_flush(VIRT_ADDR_TMP_PAGE);
	pg_table = (uint32_t *)VIRT_ADDR_TMP_PAGE;
	if (!(pg_table[tmp] & PAGE_PRESENT) || !(pg_table[tmp] & PAGE_USER))
		goto out;
	ret = pg_table[tmp] & VIRT_ADDR_FRAME_MASK;
 out:
	tmp_map_page(old);
	tlb_flush(VIRT_ADDR_TMP_PAGE);
	return ret;
}

static void
free_page_table(uint32_t pg_table)
{
	uint32_t *p, old;

	old = get_tmp_page();
	tmp_map_page(pg_table);
	tlb_flush(VIRT_ADDR_TMP_PAGE);
	p = (uint32_t *)VIRT_ADDR_TMP_PAGE;
	while (p < (uint32_t *)VIRT_ADDR_TMP_PAGE + (PAGE_SIZE / sizeof(uint32_t))) {
		if (*p & PAGE_PRESENT) {
			*p &= ~PAGE_PRESENT;
			page_frame_free(*p & 0xFFFFF000);
		}
		p++;
	}
	tmp_map_page(old);
	tlb_flush(VIRT_ADDR_TMP_PAGE);
}

void
virt_free_chunk(uint32_t start, uint32_t len, uint32_t *pg_dir)
{
	uint32_t old, *p, pg_table;
	int nr_tables;

	if (start >= KERNEL_VIRT_BASE)
		panic("virt_free_chunck: trying to free kernel memory!");
	if (pg_dir == NULL)
		panic("virt_free_chunk: pg_dir is NULL");
	old = get_tmp_page();
	tmp_map_page((uint32_t)pg_dir);
	tlb_flush(VIRT_ADDR_TMP_PAGE);
	nr_tables = len / 0x400000 + 1;
	p = ((uint32_t *)VIRT_ADDR_TMP_PAGE) + (start >> VIRT_ADDR_PG_DIR_SHIFT);
	while (nr_tables--) {
		if (*p & PAGE_PRESENT) {
			pg_table = *p & 0xFFFFF000;
			free_page_table(pg_table);
			page_frame_free(pg_table);
		}
		p++;
	}
	tmp_map_page(old);
	tlb_flush(VIRT_ADDR_TMP_PAGE);
}

void
free_address_space(struct exec_image *image)
{
	if (image->e_data_size > 0)
		virt_free_chunk(image->e_data_vaddr, image->e_data_size, (uint32_t *)current_cpu_state->cr3);
	if (image->e_rodata_size > 0)
		virt_free_chunk(image->e_rodata_vaddr, image->e_rodata_size, (uint32_t *)current_cpu_state->cr3);
	if (image->e_text_size > 0)
		virt_free_chunk(image->e_text_vaddr, image->e_text_size, (uint32_t *)current_cpu_state->cr3);
}

/*
 * 'exec' should still be able to return if something
 * goes wrong, so we can't overwrite the existing page directory
 * until we are sure everything is okay.
 * Also try to allocate the stack at top of user memory.
 */
int
arch_valloc_segments(struct exec_image *image)
{
	uint32_t *pg_dir, old;

	pg_dir = (uint32_t *)page_frame_alloc();
	if ((uint32_t)pg_dir == NO_FREE_PAGE)
		return -1;
	old = get_tmp_page();
	tmp_map_page((uint32_t)pg_dir);
	tlb_flush(VIRT_ADDR_TMP_PAGE);
	memmove((void *)VIRT_ADDR_TMP_PAGE, (void *)init_page_directory, PAGE_SIZE);
	if (image->e_text_vaddr > KERNEL_VIRT_BASE)
		return -1;
	if (image->e_text_size > 0) {
		if (virt_map_chunk(image->e_text_vaddr, image->e_text_size, pg_dir, PAGE_USER, 0) == 0)
			return -1;
	}
	if (image->e_data_vaddr > KERNEL_VIRT_BASE)
		return -1;
	if (image->e_data_size > 0) {
		if (virt_map_chunk(image->e_data_vaddr, image->e_data_size, pg_dir, PAGE_USER | PAGE_WRITABLE, 0) == 0)
			return -1;
	}
	if (image->e_rodata_size > 0) {
		if (virt_map_chunk(image->e_rodata_vaddr, image->e_rodata_size, pg_dir, PAGE_USER, 0) == 0)
			return -1;
	}
	if (virt_map_chunk(KERNEL_VIRT_BASE - USER_STACK_SIZE, USER_STACK_SIZE, pg_dir, PAGE_USER | PAGE_WRITABLE, 0) == 0)
		return -1;
	current_cpu_state->next_cr3 = (uint32_t)pg_dir;
	tmp_map_page(old);
	tlb_flush(VIRT_ADDR_TMP_PAGE);
	return 0;
}

void
copy_page_table(uint32_t *to, uint32_t *from)
{
	uint32_t old;
	char tmp_page[PAGE_SIZE];

	old = get_tmp_page();
	tmp_map_page((uint32_t)from);
	tlb_flush(VIRT_ADDR_TMP_PAGE);
	memmove(tmp_page, (void *)VIRT_ADDR_TMP_PAGE, PAGE_SIZE);
	tmp_map_page((uint32_t)to);
	tlb_flush(VIRT_ADDR_TMP_PAGE);
	memmove((void *)VIRT_ADDR_TMP_PAGE, tmp_page, PAGE_SIZE);
	tmp_map_page(old);
	tlb_flush(VIRT_ADDR_TMP_PAGE);
}

void
handle_page_fault(uint32_t error)
{
	uint32_t addr, pg_table, pg_frame, old, new, *ptr;
	struct exec_image *image;

	if (!(error & PF_PROTECTION)) {
		printk("Segmentation fault: PF_PROTECTION\r\n");
		kernel_exit(SIGSEGV);
	}
	if (!(error & PF_WRITE)) {
		printk("Segmentation fault: PF_WRITE\r\n");
		kernel_exit(SIGSEGV);
	}
	addr = get_faulting_addr();
	image = &current_process->image;
	/*
	 * Now check if faulting address is within read-only memory.
	 * It it is, then segfault because that is not allowed.
	 */
	if (addr >= image->e_text_vaddr && addr < image->e_text_vaddr + image->e_text_size) {
		printk("Segmentation fault\r\n");
		kernel_exit(SIGSEGV);
	}
	if (image->e_rodata_size > 0) {
		if (addr >= image->e_rodata_vaddr && addr < image->e_rodata_vaddr + image->e_rodata_size) {
			printk("Segmentation fault\r\n");
			kernel_exit(SIGSEGV);
		}
	}
	if (page_get_count(addr & 0xFFFFF000) > 1) {
		page_set_writable(addr, 1);
	} else {
		page_set_writable(addr, 0);
	}
}

static void
page_set_writable(uintptr_t addr, int copy)
{
	uint32_t old, new, *ptr, pg_frame, pg_table;
	char tmp_page[PAGE_SIZE];
	int old_flags;
	
	new = 0;
	old = get_tmp_page();
	tlb_flush(VIRT_ADDR_TMP_PAGE);
	tmp_map_page(current_cpu_state->cr3);
	tlb_flush(VIRT_ADDR_TMP_PAGE);
	pg_table = ((uint32_t *)VIRT_ADDR_TMP_PAGE)[addr >> VIRT_ADDR_PG_DIR_SHIFT];
	if (!(pg_table & PAGE_PRESENT)) {
		printk("Segmentation fault\r\n");
		kernel_exit(SIGSEGV);
	}
	((uint32_t *)VIRT_ADDR_TMP_PAGE)[addr >> VIRT_ADDR_PG_DIR_SHIFT] |= PAGE_WRITABLE;
	tmp_map_page(pg_table & 0xFFFFF000);
	tlb_flush(VIRT_ADDR_TMP_PAGE);
	ptr = &((uint32_t *)VIRT_ADDR_TMP_PAGE)[(addr >> VIRT_ADDR_PG_TAB_SHIFT) & VIRT_ADDR_PG_TAB_MASK];
	pg_frame = *ptr;
	old_flags = pg_frame & 0xFFF;
	if (!(pg_frame & PAGE_PRESENT)) {
		if (new != 0)
			page_frame_free(new);
		printk("Segmentation fault\r\n");
		kernel_exit(SIGSEGV);
	}
	if (copy) {
		new = page_frame_alloc();
		if (new == NO_FREE_PAGE)
			panic("Out of memory");
		page_frame_free(*ptr & 0xFFFFF000);
		*ptr = new | old_flags | PAGE_WRITABLE;
		page_decrease_count(addr & 0xFFFFF000);
		tmp_map_page(pg_frame & 0xFFFFF000);
		tlb_flush(VIRT_ADDR_TMP_PAGE);
		memmove(tmp_page, (void *)VIRT_ADDR_TMP_PAGE, PAGE_SIZE);
		tmp_map_page(new);
		tlb_flush(VIRT_ADDR_TMP_PAGE);
		memmove((void *)VIRT_ADDR_TMP_PAGE, tmp_page, PAGE_SIZE);
	} else {
		*ptr |= PAGE_WRITABLE;
	}
	tlb_flush(addr);
	tmp_map_page(old);
	tlb_flush(VIRT_ADDR_TMP_PAGE);
}

