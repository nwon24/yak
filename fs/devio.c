/*
 * devio.c
 * Handles device IO request using major device numbers.
 */
#include <stddef.h>

#include <drivers/drive.h>

#include <generic/errno.h>

#include <fs/buffer.h>
#include <fs/dev.h>

static int hddev_physio(struct buffer *bp, int rw);

static struct blk_dev_ops blk_dev_table[] = {
	[NODEV]  = { .d_rw = NULL },
	[RD_DEV] = { .d_rw = NULL },
	[FD_DEV] = { .d_rw = NULL },
	[HD_DEV] = { .d_rw = hddev_physio }
};

static struct chr_dev_ops chr_dev_table[] = {
	[TTY_DEV] = { .d_open = tty_open, .d_rw = tty_rw, .d_close = tty_close },
	[TTYX_DEV] = { .d_open = tty_open, .d_rw = ttyx_rw, .d_close = tty_close},
};

int
blk_devio(struct buffer *bp, int rw)
{
	if (blk_dev_table[DEV_MAJOR(bp->b_dev)].d_rw)
		return (*blk_dev_table[DEV_MAJOR(bp->b_dev)].d_rw)(bp, rw);
	bp->b_flags |= B_ERROR;
	return -ENODEV;
}

int
chr_devio(dev_t dev, char *buf, int count, int rw)
{
	int major = DEV_MAJOR(dev);

	if (!is_chardev(dev))
		return -ENODEV;
	if (chr_dev_table[major].d_rw)
		return (*chr_dev_table[major].d_rw)(DEV_MINOR(dev), buf, count, rw);
	return -ENODEV;
}

int chr_dev_open(dev_t dev)
{
	if (!is_chardev(dev))
		return -1;
	return (*chr_dev_table[DEV_MAJOR(dev)].d_open)(DEV_MINOR(dev));
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
