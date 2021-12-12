#ifndef _PAGING_H
#define _PAGING_H

#ifdef _KERNEL_
#include <stdint.h>

#include <mm/mmap.h>

/* External symbols from linker script. */
extern uint32_t _start_kernel;
extern uint32_t _end_kernel;

uint32_t kernel_virt_map_page(uint32_t page_frame);

#define kernel_virt_put_page()	(kernel_virt_map_page(page_frame_alloc()))

#endif /* _KERNEL */

#include <kernel/config.h>

#define KERNEL_LOAD_PHYS_ADDR	0x100000
#define KERNEL_LOAD_VIRT_ADDR	(KERNEL_LOAD_PHYS_ADDR + KERNEL_VIRT_BASE)

#ifdef _ASSEMBLY_
#define ASM_PHYS_ADDR(x)	(x - KERNEL_VIRT_BASE)
#else
#define PHYS_ADDR(x)		(((uint32_t)(x)) - KERNEL_VIRT_BASE)
#define VIRT_ADDR(x)		(((uint32_t)(x)) + KERNEL_VIRT_BASE)
#endif /* _ASSEMBLY_ */

#define PAGE_SIZE		4096
#define PAGE_SHIFT		12	/* 2^12 = 4096 */

#define PAGE_PRESENT		0x1
#define PAGE_WRITABLE		0x2
#define PAGE_SUPERVISOR		0
#define PAGE_USER		0x4

#define PG_ENABLE		0x80010000

/* Page directory entry is upper 10 bits of 32 bit virtual address */
#define VIRT_ADDR_PG_DIR_SHIFT	22
/* Page table entry is middle 10 bits of 32 bit virtual address */
#define VIRT_ADDR_PG_TAB_SHIFT	12
/* Mask to get rid of upper 10 bits of 32 bit virtual address, to leave just the page table index */
#define VIRT_ADDR_PG_TAB_MASK	0x3FF
/* Page frame offset is lower 12 bits of 32 bit virtual address */
#define VIRT_ADDR_FRAME_MASK	0xFFF

#define VIRT_ADDR_PG_DIR_INDEX(x)	((x) >> VIRT_ADDR_PG_DIR_SHIFT)
#define VIRT_ADDR_PG_TAB_INDEX(x)	(((x) >> VIRT_ADDR_PG_TAB_SHIFT) & VIRT_ADDR_PG_TAB_MASK)
#define VIRT_ADDR_FRAME_OFFSET(x)	((x) & VIRT_ADDR_FRAME_MASK)

#ifndef _ASSEMBLY_
#define IS_PAGE_ALIGNED(x)	(!((x) & VIRT_ADDR_FRAME_MASK))
#endif /* _ASSEMBLY_ */

#endif /* _PAGING_H */
