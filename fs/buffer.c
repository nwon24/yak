/*
 * buffer.c
 * Buffer cache functions.
 */
#include <stddef.h>

#include <asm/types.h>
#include <asm/interrupts.h>

#include <fs/buffer.h>
#include <fs/dev.h>
#include <fs/fs.h>

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
static void put_into_free_list(struct buffer *bp);
static void put_into_hash_queue(struct buffer *bp);

struct buffer *
getblk(dev_t dev, size_t blknr)
{
	struct buffer *bp = NULL;

	while (1) {
		if ((bp = in_hash_queue(dev, blknr)) != NULL) {
			if (bp->b_mutex == MUTEX_LOCKED) {
				sleep(bp);
				continue;
			}
			mutex_lock(&bp->b_mutex);
			remove_from_free_list(bp);
			return bp;
		}
		if ((bp = free_list.b_next_free) == NULL) {
			/* Nothing on free list */
			sleep(&free_list);
			continue;
		}
		remove_from_free_list(bp);
		if (bp->b_flags & B_DWRITE) {
			bp->b_flags &= ~B_DWRITE;
			bwrite(bp);
			continue;
		}
		mutex_lock(&bp->b_mutex);
		bp->b_dev = dev;
		bp->b_blknr = blknr;
		bp->b_blksize = filesystem_get_attr(dev, GET_BLOCKSIZE);
		if (bp->b_blksize)
			bp->b_data = kvmalloc(bp->b_blksize);
		bp->b_flags = 0;
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
		if (bp == free_list.b_next_free + NR_BUF_BUFFERS - 1)
			bp->b_next_free = &free_list;
		else
			bp->b_next_free = bp + 1;
		bp->b_next = NULL;
		bp->b_prev = NULL;
		bp->b_flags = 0;
	}
	free_list.b_prev_free = bp;
}

void
brelse(struct buffer *bp)
{
	disable_intr();
	put_into_free_list(bp);
	enable_intr();
	if (bp->b_data != NULL) {
		kvmfree(bp->b_data);
		/* So we can't write to it by accident */
		bp->b_data = NULL;
	}
	mutex_unlock(&bp->b_mutex);
	wakeup(bp);
	wakeup(&free_list);
}

void
bwrite(struct buffer *bp)
{
	blk_devio(bp, WRITE);
	brelse(bp);
}

struct buffer *
bread(dev_t dev, size_t blknr)
{
	struct buffer *bp;

	bp = getblk(dev, blknr);
	if (bp->b_flags & B_DONE)
		return bp;
	blk_devio(bp, READ);
	return bp;
}

static struct buffer *
in_hash_queue(dev_t dev, size_t blknr)
{
	struct buffer *bp, *tmp;

	bp = hash_queues[BUF_HASH(dev, blknr)];
	if (bp == NULL)
		return NULL;
	tmp = bp;
	do {
		if (bp->b_dev == dev && bp->b_blknr == blknr)
			return bp;
		bp = bp->b_next;
	} while (bp != tmp);
	return NULL;
}

static void
remove_from_free_list(struct buffer *bp)
{
	if (bp->b_prev_free == NULL && bp->b_next_free == NULL)
		/* Not on free list */
		return;
	if (bp->b_prev_free == NULL || bp->b_next_free == NULL)
		panic("remove_from_free_list: free list has been corrupted");
	bp->b_prev_free->b_next_free = bp->b_next_free;
	bp->b_next_free->b_prev_free = bp->b_prev_free;
}

static void
remove_from_hash_queue(struct buffer *bp)
{
	if (bp->b_prev == NULL && bp->b_next == NULL)
		/* Not on a hash queue */
		return;
	if (bp->b_prev == NULL || bp->b_next == NULL)
		panic("remove_from_hash_queue: bp is not on a hash queue or queue has been corrupted!");
	if (bp->b_prev == bp && bp->b_next == bp) {
		hash_queues[BUF_HASH(bp->b_dev, bp->b_blknr)] = NULL;
	} else {
		bp->b_prev->b_next = bp->b_next;
		bp->b_next->b_prev = bp->b_prev;
	}
	bp->b_prev = bp->b_next = NULL;
}

static void
put_into_hash_queue(struct buffer *bp)
{
	struct buffer *tp1, *tp2;

	if ((tp1 = hash_queues[BUF_HASH(bp->b_dev, bp->b_blknr)]) == NULL) {
		hash_queues[BUF_HASH(bp->b_dev, bp->b_blknr)] = bp;
		bp->b_next = bp;
		bp->b_prev = bp;
		return;
	}
	for (tp2 = tp1; tp2->b_next != tp1; tp2 = tp2->b_next);
	tp2->b_next = bp;
	bp->b_prev = tp2;
	bp->b_next = tp1;
	tp1->b_prev = bp;
}

static void
put_into_free_list(struct buffer *bp)
{
	if (!(bp->b_flags & B_DWRITE) && !(bp->b_flags & B_ERROR)) {
		/*
		 * Contents are valid and there has bee no error.
		 * Put it at the end of the free list.
		 */
		bp->b_prev_free = free_list.b_prev_free;
		bp->b_next_free = &free_list;
		free_list.b_prev_free->b_next_free = bp;
		free_list.b_prev_free = bp;
	} else {
		/*
		 * Buffer contents are old.
		 * Put it at the start of the free list.
		 */
		bp->b_prev_free = &free_list;
		bp->b_next_free = free_list.b_next_free;
		free_list.b_next_free->b_prev_free = bp;
		free_list.b_next_free = bp;
	}
}
