/*
 * kvmalloc.c
 * Implementation of malloc() for kernel use.
 * Memory returned will be virtual address in the higher half (>3GIB)
 * This memory allocator simply uses a small header before each free
 * chunk of memory. Allocating memory is simply a matter ot traversing the free list
 * to find a suitably sized block. Freeing consists of finding where the block
 * should go in the list and then inserting it there, mergeing blocks if contiguous.
 */

#include <stddef.h>

#include <asm/paging.h>

#include <kernel/debug.h>

#include <mm/mm.h>
#include <mm/vm.h>

static struct vm_chunk_header *free_list;

static void merge_blocks(struct vm_chunk_header *ptr);
static void remove_from_list(struct vm_chunk_header *ptr);
static void insert_before(struct vm_chunk_header *p, struct vm_chunk_header *before);
static struct vm_chunk_header *expand_heap(void);

/*
 * Takes the address of a heap and its size and initialises the free linked list
 * of memory.
 */
void
kvmalloc_init(uint32_t heap, uint32_t size)
{
	free_list = (struct vm_chunk_header *)heap;
	free_list->base = heap + sizeof(free_list);
	free_list->size = size - sizeof(free_list);
	free_list->next = NULL;
	free_list->prev = NULL;
}

void *
kvmalloc(size_t bytes)
{
	struct vm_chunk_header *ptr, *tmp;
	void *p;

	for (ptr = free_list; ptr != NULL; ptr = ptr->next) {
		if (ptr->size >= bytes) {
			if (ptr->size == bytes) {
				p = (void *)ptr->base;
				remove_from_list(ptr);
			} else {
				ptr->size -= bytes + sizeof(*ptr);
				p = (void *)(ptr->base + ptr->size + sizeof(*ptr));
				tmp = (struct vm_chunk_header *)(ptr->base + ptr->size);
				tmp->size = bytes;
				tmp->base = (uint32_t)p;
			}
			return p;
		}
	}
	return NULL;
}

void
kvmfree(void *p)
{
	struct vm_chunk_header *ptr, *tmp;

	for (ptr = free_list; ; ptr = ptr->next) {
		if (ptr->base > (uint32_t)p) {
			insert_before(p, ptr);
			merge_blocks(p);
			return;
		}
		if (ptr->next == NULL)
			break;
	}
	tmp = (struct vm_chunk_header *)p - 1;
	ptr->next = tmp;
	printk("Here\r\n");
	merge_blocks(ptr);
}

static void
remove_from_list(struct vm_chunk_header *ptr)
{
	if (ptr->next == NULL && ptr->prev == NULL) {
		/* Only element in the list */
		free_list = expand_heap();
	} else {
		if (ptr->prev)
			ptr->prev->next = ptr->next;
		if (ptr->next)
			ptr->next->prev = ptr->prev;
	}
}

static struct vm_chunk_header *
expand_heap(void)
{
	uint32_t tmp, ret;
	int i;

	/* Just to make the compiler happy */
	ret = 0;

	for (i = 0; i < HEAP_SIZE / PAGE_SIZE; i++) {
		if (!(tmp = kernel_virt_put_page()))
			panic("Unable to expand heap, out of memory");
		if (!i)
			ret = tmp;
	}
	return (struct vm_chunk_header *)ret;
}

static void
insert_before(struct vm_chunk_header *p, struct vm_chunk_header *before)
{
	if (before->prev) {
		before->prev->next = p;
		p->prev = before->prev;
	}
	before->prev = p;
	p->next = before;
}

static void
merge_blocks(struct vm_chunk_header *ptr)
{
	if (ptr->prev) {
		if (ptr->prev->base + ptr->prev->size == ptr->base - sizeof(*ptr)) {
			ptr->prev->size += ptr->size + sizeof(*ptr);
			ptr->prev->next = ptr->next;
		}
	}
	if (ptr->next) {
		if (ptr->base + ptr->size == (uint32_t)ptr->next) {
			ptr->size += ptr->next->size + sizeof(*ptr);
			ptr->next = ptr->next->next;
		}
	}
}
