/*
 * page_alloc.c
 * Page frame allocator.
 * All addresses are physical addresses.
 * Fairly straightforward stuff. Just need to be careful with virtual addresses.
 */

#include <stddef.h>
#include <stdint.h>

#include <asm/paging.h>

#include <kernel/debug.h>

#include <mm/mm.h>
#include <mm/mmap.h>

static char reference_counts[MAX_MEM / PAGE_SIZE];

static void set_bit(uint8_t *bitmap, uint32_t bit);
static void clear_bit(uint8_t *bitmap, uint32_t bit);
static void scan_bitmap(struct mmap_bitmap *bmap);

uint32_t
page_frame_alloc(void)
{
	struct mmap_bitmap *bmap;
	uint32_t ret;
	uint8_t *ptr;

	for (bmap = mmap_bitmaps; bmap->mmap_entry->next != NULL; bmap++) {
		if (bmap->free_pages)
			break;
	}
	if (!bmap->free_pages) {
		if (bmap->next_free != NO_FREE_PAGE)
			panic("Memory map bitmaps corrupted");
		return NO_FREE_PAGE;
	}
	ret = bmap->next_free * PAGE_SIZE + bmap->mmap_entry->base;
	ptr = (uint8_t *)VIRT_ADDR(bmap->base);
	set_bit(ptr, bmap->next_free);
	bmap->free_pages--;
	if (bmap->free_pages)
		scan_bitmap(bmap);
	else
		bmap->next_free = NO_FREE_PAGE;
	reference_counts[ret / PAGE_SIZE] = 1;
	return ret;
}

void
page_frame_free(uint32_t page)
{
	struct mmap_bitmap *bmap;
	uint32_t bit;
	uint8_t *ptr;

	if (--reference_counts[page / PAGE_SIZE] > 0)
		return;
	if (!IS_PAGE_ALIGNED(page))
		panic("page_frame_free: page has wrong alignment");
	reference_counts[page / PAGE_SIZE]--;
	if (reference_counts[page / PAGE_SIZE])
		return;
	for (bmap = mmap_bitmaps; bmap->mmap_entry->next != NULL; bmap++) {
		if (bmap->mmap_entry->base + bmap->mmap_entry->size > page)
			/* Found correct bitmap */
			break;
	}
	if (bmap->mmap_entry->base + bmap->mmap_entry->size < page)
		panic("page_frame_free: page address is nowhere in memory!");
	page -= bmap->mmap_entry->base;
	bit = page / PAGE_SIZE;
	ptr = (uint8_t *)VIRT_ADDR(bmap->base);
	clear_bit(ptr, bit);
	bmap->free_pages++;
	if (bit < bmap->next_free)
		bmap->next_free = bit;
}

void
page_increase_count(uint32_t page)
{
	reference_counts[page / PAGE_SIZE]++;
}

void
page_decrease_count(uintptr_t page)
{
	reference_counts[page / PAGE_SIZE]--;
}

int
page_get_count(uintptr_t page)
{
	return reference_counts[page / PAGE_SIZE];
}

static void
set_bit(uint8_t *bitmap, uint32_t bit)
{
	int ind, off;

	ind = bit >> 3;
	off = bit % 8;
	*(bitmap + ind) |= (1 << off);
}

static void
clear_bit(uint8_t *bitmap, uint32_t bit)
{
	int ind, off;

	ind = bit >> 3;
	off = bit % 8;
	*(bitmap + ind) &= ~(1 << off);
}

static void
scan_bitmap(struct mmap_bitmap *bmap)
{
	int i, j;
	uint8_t *map;

	i = bmap->next_free >> 3;
	j = bmap->next_free % 8;
	map = (uint8_t *)VIRT_ADDR(bmap->base);
	while (map[i] & (1 << j)) {
		if ((uint32_t)(i << 3) + j == bmap->size) {
			bmap->next_free = NO_FREE_PAGE;
			return;
		}
		if (j == 7) {
			j = 0;
			i++;
		} else {
			j++;
		}
	}
	bmap->next_free = (i << 3) + j;
}
