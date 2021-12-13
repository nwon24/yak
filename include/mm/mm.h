#ifndef _MM_H
#define _MM_H

#include <stdint.h>

#include <generic/multiboot.h>

void mm_init(void);

uint32_t page_frame_alloc(void);
void page_frame_free(uint32_t page);
/*
 * Since all page frames must be 4 KiB aligned, a value of 1 can mean an invalid page.
 */
#define NO_FREE_PAGE	1

#endif /* _MM_H */
