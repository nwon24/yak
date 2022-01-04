/*
 * buffer.c
 * Buffer cache functions.
 */
#include <stddef.h>

#include <asm/types.h>

#include <fs/buffer.h>
#include <fs/dev.h>
#include <fs/misc.h>

#include <kernel/mutex.h>
#include <kernel/debug.h>
#include <kernel/proc.h>

#include <mm/vm.h>

#define BUF_HASH(dev, blknr)	(((dev) ^ (blknr)) % NR_BUF_HASH)

/*
 * A dummy buffer that points to the start of the free list.
 */
static struct buffer free_list;
static struct buffer **hash_queues;

static struct buffer *in_hash_queue(dev_t dev, size_t blknr);
static void remove_from_free_list(struct buffer *bp);
static void remove_from_hash_queue(struct buffer *bp);
static void put_into_hash_queue(struct buffer *bp);
static void bwrite(struct buffer *bp);

struct buffer *
get_block(dev_t dev, size_t blknr)
{
	struct buffer *bp = NULL;

	while (bp == NULL) {
		if ((bp = in_hash_queue(dev, blknr)) != NULL) {
			mutex_lock(&bp->b_mutex);
			return bp;
		}
		if ((bp = &free_list)->b_next_free == NULL) {
			/* Nothing on free list */
			sleep(&free_list);
			continue;
		}
		remove_from_free_list(bp);
		if (bp->b_flags & B_DWRITE) {
			bwrite(bp);
			continue;
		}
		mutex_lock(&bp->b_mutex);
		bp->b_dev = dev;
		bp->b_blknr = blknr;
		remove_from_hash_queue(bp);
		put_into_hash_queue(bp);
		return bp;
	}
	return bp;
}

void
buffer_init(void)
{
	struct buffer *bp;
	int i;

	/*
	 * Dynamically allocate buffer cache so one day it can be resized,
	 * if we ever implement kvmrealloc().
	 */
	hash_queues = kvmalloc(NR_BUF_HASH * sizeof(*hash_queues));
	if (hash_queues == NULL)
		panic("Unable to allocate hash table: kvmalloc returned NULL");
	for (i = 0; i < NR_BUF_HASH; i++)
		hash_queues[i] = NULL;
	free_list.b_next_free = kvmalloc(NR_BUF_BUFFERS * sizeof(*free_list.b_next_free));
	if (free_list.b_next_free == NULL)
		panic("Unable to allocate buffer cache: kvmalloc returned NULL");
	for (bp = free_list.b_next_free; bp < free_list.b_next_free + NR_BUF_BUFFERS; bp++) {
		bp->b_dev = NODEV;
		if (bp == free_list.b_next_free)
			bp->b_prev_free = &free_list;
		else
			bp->b_prev_free = bp - 1;
		if (bp == free_list.b_next_free + NR_BUF_BUFFERS)
			bp->b_next_free = &free_list;
		else
			bp->b_next_free = bp + 1;
		bp->b_next = NULL;
		bp->b_prev = NULL;
		bp->b_flags = 0;
	}
}

static void
bwrite(struct buffer *bp)
{
	blk_devio(bp, WRITE);
}

static struct buffer *
in_hash_queue(dev_t dev, size_t blknr)
{
	struct buffer *bp;

	bp = hash_queues[BUF_HASH(dev, blknr)];
	while (bp != NULL) {
		if (bp->b_dev == dev && bp->b_blknr == blknr)
			return bp;
		bp = bp->b_next;
	}
	return NULL;
}

static void
remove_from_free_list(struct buffer *bp)
{
	if (bp->b_prev_free == NULL || bp->b_next_free == NULL)
		panic("remove_from_free_list: bp is not on the free list or list has been corrupted!");
	bp->b_prev_free->b_next_free = bp->b_next_free;
	bp->b_next_free->b_prev_free = bp->b_prev_free;
}

static void
remove_from_hash_queue(struct buffer *bp)
{
	if (bp->b_prev == NULL || bp->b_next == NULL)
		panic("remove_from_hash_queue: bp is not on a hash queue or queue has been corrupted!");
	bp->b_prev->b_next = bp->b_next;
	bp->b_next->b_prev = bp->b_prev;
}

static void
put_into_hash_queue(struct buffer *bp)
{
	struct buffer *tp1, *tp2;

	if ((tp1 = hash_queues[BUF_HASH(bp->b_dev, bp->b_blknr)]) == NULL) {
		hash_queues[BUF_HASH(bp->b_dev, bp->b_blknr)] = bp;
		bp->b_next = bp;
		bp->b_prev = bp;
	}
	for (tp2 = tp1; tp2->b_next != tp1; tp2 = tp2->b_next);
	tp2->b_next = bp;
	bp->b_prev = tp2;
	bp->b_next = tp1;
	tp1->b_prev = bp;
}
