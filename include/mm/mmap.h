#ifndef MMAP_H
#define MMAP_H

#include <stdint.h>

#define MAX_MMAP_ENTRIES	16

struct mmap_entry {
	uint32_t base;			/* Base address of memory */
	uint32_t size;			/* Size of region */
	uint32_t bitmap_size;		/* Size of bitmap, in bytes */
	struct mmap_entry *next;	/* Next entry */
};

struct mmap_bitmap {
	struct mmap_entry *mmap_entry;
	uint32_t base;
	uint32_t size;
	uint32_t next_free;
	uint32_t free_pages;
};

extern struct mmap_entry mmap[];
extern struct mmap_bitmap mmap_bitmaps[];

extern uint32_t total_mem;

#endif /* MMAP_H */
