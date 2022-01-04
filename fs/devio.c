/*
 * devio.c
 * Handles device IO request using major device numbers.
 */
#include <stddef.h>

#include <drivers/drive.h>

#include <fs/buffer.h>
#include <fs/dev.h>

static int hddev_physio(struct buffer *bp, int rw);

static struct blk_dev_ops blk_dev_table[] = {
	[NODEV]  = { .d_rw = NULL },
	[RD_DEV] = { .d_rw = NULL },
	[FD_DEV] = { .d_rw = NULL },
	[HD_DEV] = { .d_rw = hddev_physio }
};

int
blk_devio(struct buffer *bp, int rw)
{
	if (blk_dev_table[DEV_MAJOR(bp->b_dev)].d_rw)
		return (*blk_dev_table[DEV_MAJOR(bp->b_dev)].d_rw)(bp, rw);
	bp->b_flags |= B_ERROR;
	return -1;
}

static int
hddev_physio(struct buffer *bp, int rw)
{
	int ratio = bp->b_blksize / SECTOR_SIZE, ret;
	size_t count = ratio, block_off;

	block_off = bp->b_blknr * ratio;
	if (drive_rw(DEV_MINOR(bp->b_dev), block_off, bp->b_data, count, rw) < 0) {
		bp->b_flags |= B_ERROR;
		ret = -1;
	} else {
		bp->b_flags |= B_DONE;
		ret = 0;
	}
	return ret;
}
