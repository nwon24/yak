#ifndef VM_H
#define VM_H

#include <stdint.h>
#include <stddef.h>

#define HEAP_SIZE	0x400000

/*
 * Header of a chunk of virtual memory.
 * 'base' is the virtual address.
 */
struct vm_chunk_header {
	uint32_t base;
	uint32_t size;
	struct vm_chunk_header *next;
	struct vm_chunk_header *prev;
};

void vm_init(void);
void kvmalloc_init(uint32_t heap, uint32_t size);
void *kvmalloc(size_t bytes);
void kvmfree(void *p);

#endif /* VM_H */
