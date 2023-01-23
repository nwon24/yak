#ifndef PAGING_H
#define PAGING_H

#ifdef KERNEL
#include <stdint.h>

#include <kernel/config.h>
#include <kernel/exec.h>

#include <mm/mmap.h>

#define MAX_MEM	0xFFFFFFFF

/* External symbols from linker script. */
extern uint32_t _start_kernel;
extern uint32_t _end_kernel;

uint32_t kernel_virt_map_page(uint32_t page_frame);
uint32_t virt_map_phys(uint32_t phys);
void virt_unmap_virt(uint32_t virt);
uint32_t virt_map_chunk(uint32_t start, uint32_t len, uint32_t *pg_dir, int flags, uint32_t phys);
void virt_free_chunk(uint32_t start, uint32_t len, uint32_t *pg_dir);
void copy_address_space(uint32_t from_pg_dir, uint32_t to_pg_dir);
int check_user_ptr(void *addr);
void free_address_space(struct exec_image *image);
int arch_valloc_segments(struct exec_image *image);
void copy_page_table(uint32_t *to_pg_dir, uint32_t *from_pg_dir);
void handle_page_fault(uint32_t error);

extern uint32_t current_page_directory;

#define kernel_virt_put_page()	(kernel_virt_map_page(page_frame_alloc()))

/* 2 MiB stack */
#define USER_STACK_SIZE	0x200000

#ifdef CONFIG_USE_INLINE_ASM
static inline void
tlb_flush(uint32_t page)
{
#ifdef CONFIG_X86_ISA_I686
        __asm__("invlpg (%0)" : : "r" (page) : "memory");
#else
        __asm__("movl %%cr3, %%eax\r\n"
                "movl %%eax, %%cr3\r\n" : : "%eax");
#endif /* CONFIG_X86_ISA_I686 */
}

static inline uint32_t
get_faulting_addr(void)
{
	uint32_t ret;

	__asm__("movl %%cr2, %0\r\n" : "=r" (ret) :);
	return ret;
}

static inline void
load_cr3(uint32_t pg_dir)
{
	__asm__("movl %0, %%cr3" : : "r" (pg_dir));
}

static inline void
reload_cr3(void)
{
	__asm__ volatile("movl %%cr3, %%eax\n\t"
                "movl %%eax, %%cr3\n\t" : : : "%eax");
}
#else
void tlb_flush(uint32_t page);
void load_cr3(uint32_t cr3);
uint32_t get_faulting_addr(void);
void reload_cr3(void);
#endif /* CONFIG_USE_INLINE_ASM */

#endif /* KERNEL */

#include <kernel/config.h>

#define KERNEL_LOAD_PHYS_ADDR	0x100000
#define KERNEL_LOAD_VIRT_ADDR	(KERNEL_LOAD_PHYS_ADDR + KERNEL_VIRT_BASE)

#ifdef __ASSEMBLER__
#define ASM_PHYS_ADDR(x)	(x - KERNEL_VIRT_BASE)
#else
#define PHYS_ADDR(x)		(((uint32_t)(x)) - KERNEL_VIRT_BASE)
#define VIRT_ADDR(x)		(((uint32_t)(x)) + KERNEL_VIRT_BASE)
#endif /* __ASSEMBLER__ */

#define PAGE_SIZE		4096
#define PAGE_SHIFT		12	/* 2^12 = 4096 */

#define PAGE_PRESENT		0x1
#define PAGE_WRITABLE		0x2
#define PAGE_SUPERVISOR		0
#define PAGE_USER		0x4

#define PG_ENABLE		0x80000001

#define PF_PROTECTION		1
#define PF_WRITE		2
#define PF_USER			4

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

#ifndef __ASSEMBLER__
#define IS_PAGE_ALIGNED(x)	(!((x) & VIRT_ADDR_FRAME_MASK))
#endif /* __ASSEMBLER__ */

#endif /* PAGING_H */
