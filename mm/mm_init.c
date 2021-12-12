/*
 * mm_init.c
 * Initialises the internal memory map.
 */

#include <stddef.h>

#include <asm/paging.h>

#include <generic/multiboot.h>

#include <kernel/debug.h>

#include <mm/mmap.h>

/* Total memory of the machine */
uint32_t total_mem = 0;
/* Physical addresses of kernel start and end */
uint32_t kstart_addr, kend_addr;

struct mmap_entry mmap[MAX_MMAP_ENTRIES];
struct mmap_bitmap mmap_bitmaps[MAX_MMAP_ENTRIES];

static struct mmap_entry *enter_into_mmap(uint32_t addr, uint32_t size);
static void reserve_bitmaps(void);

/*
 * Initialise the internal kernel memory map.
 * Calculate the total memory available.
 */
void
mm_init(multiboot_info_t *mb_info)
{
	multiboot_mmap_entry_t *entry, *end;
	struct mmap_entry *mep;
	uint32_t kernel_size;

	entry = (multiboot_mmap_entry_t *)VIRT_ADDR(mb_info->mmap_addr);
	end = (multiboot_mmap_entry_t *)VIRT_ADDR(mb_info->mmap_addr + mb_info->mmap_length);
	while (entry < end) {
		if (entry->type == MULTIBOOT_MEMORY_AVAILABLE) {
			total_mem += entry->len_low;
			if (enter_into_mmap(entry->addr_low, entry->len_low) == NULL)
				break;
		}
		entry = (multiboot_mmap_entry_t *)((char *)entry + entry->size + sizeof(entry->size));
	}
	printk("Total mem: %x\r\n", total_mem);
	/*
	 * _end_kernel and _start_kernel have no value. It is their address
	 * we want.
	 */
	kstart_addr = (uint32_t)&_start_kernel;
	kend_addr = (uint32_t)&_end_kernel;
	kend_addr = PHYS_ADDR(kend_addr);
	if (!IS_PAGE_ALIGNED(kstart_addr) || !IS_PAGE_ALIGNED(kend_addr)) {
		printk("start %x end %x\r\n", kstart_addr, kend_addr);
		panic("_end_kernel or _start_kernel are not page aligned");
	}
	kernel_size = kend_addr - kstart_addr;
	/*
	 * _start_kernel is guaranteed to be 1MiB (look at the linkerscript.
	 * So there will never be the problem where the kernel image is inside
	 * of a free area that we then have to split up.
	 */
	for (mep = mmap; mep->next != NULL; mep = mep->next) {
		if (mep->base == kstart_addr && mep->size > kernel_size) {
			mep->size -= kernel_size;
			mep->base += kernel_size;
		} else if (mep->base == kstart_addr && mep->size < kernel_size) {
			panic("Bizzare: there doesn't seem to be enough memory for the kernel to be running!");
		}
	}
	for (mep = mmap; mep->next != NULL; mep = mep->next)
		printk("Base %x, size %x\r\n", mep->base, mep->size);
	reserve_bitmaps();
}

static struct mmap_entry *
enter_into_mmap(uint32_t base, uint32_t size)
{
	struct mmap_entry *entry;

	for (entry = mmap; entry->size != 0 && entry < mmap + MAX_MMAP_ENTRIES; entry++);
	if (entry >= mmap + MAX_MMAP_ENTRIES)
		return NULL;
	entry->base = base;
	entry->size = size;
	entry->bitmap_size = size / PAGE_SIZE + 1;
	if (entry + 1 < mmap + MAX_MMAP_ENTRIES)
		entry->next = entry + 1;
	else
		entry->next = NULL;
	return entry;
}

/*
 * Calculate the number of pages needed to store bitmaps.
 */
static void
reserve_bitmaps(void)
{
	int nr;
	/* Number of bytes per page of bitmap */
	uint32_t bytes_per_page, base;
	struct mmap_entry *mep;
	struct mmap_bitmap *bmap;

	nr = 0;
	/*
	 * Each page has a certain number of bytes, each byte has 8 bits
	 * and each bit represents a page frame.
	 */
	bytes_per_page = PAGE_SIZE * 8 * PAGE_SIZE;
	nr = total_mem / bytes_per_page + 1;
	/* Set base to some impossible value for the bitmaps */
	base = 0xFFFFFFFF;
	for (mep = mmap; mep->next != NULL; mep = mep->next) {
		if (mep->size > (uint32_t) nr * PAGE_SIZE) {
			/* Found enough space for bitmaps. */
			base = mep->base;
			mep->size -= nr * PAGE_SIZE;
			mep->base += nr * PAGE_SIZE;
			break;
		}
	}
	if (base == 0xFFFFFFFF)
		panic("Not enough memory");
	for (bmap = mmap_bitmaps; bmap < mmap_bitmaps + MAX_MMAP_ENTRIES; bmap++) {
		bmap->mmap_entry = mmap + (bmap - mmap_bitmaps);
		if (bmap->mmap_entry->next == NULL)
			break;
		bmap->base = base;
		bmap->size = bmap->mmap_entry->size / PAGE_SIZE / 8 + 1;
		base += bmap->size;
	}
	for (bmap = mmap_bitmaps; bmap < mmap_bitmaps + 2; bmap++)
		printk("bitmap base %x, size %x\r\n", bmap->base, bmap->size);
}
