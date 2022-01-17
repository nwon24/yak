#ifndef BUFFER_H
#define BUFFER_H

#include <stdint.h>
#include <stddef.h>

#include <asm/types.h>

#include <kernel/config.h>
#include <kernel/mutex.h>

#define NR_BUF_HASH	CONFIG_FS_BUF_HASH_SIZE
#define NR_BUF_BUFFERS	CONFIG_FS_BUF_CACHE_SIZE

/*
 * The objects of the file system's buffer
 * cache.
 */
struct buffer {
	dev_t b_dev;
	size_t b_blknr;
	blksize_t b_blksize;
	char *b_data;
	mutex b_mutex;
	int b_flags;

	struct buffer *b_next_free;
	struct buffer *b_prev_free;
	struct buffer *b_next;
	struct buffer *b_prev;
};

#define B_DWRITE	(1 << 0)	/* Delayed write */
#define B_DONE		(1 << 1)	/* Successful completion */
#define B_ERROR		(1 << 2)	/* Error */

void buffer_init(void);
struct buffer *bread(dev_t dev, size_t blknr);
struct buffer *getblk(dev_t dev, size_t blknr);
void brelse(struct buffer *bp);
void bwrite(struct buffer *bp);
void buffer_sync(void);

#endif /* BUFFER_H */
