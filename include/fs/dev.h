#ifndef DEV_H
#define DEV_H

#define NODEV	0

/* Block device numbers */
#define RD_DEV	1
#define FD_DEV	2
#define HD_DEV	3

#define DEV_MAJOR(dev)	(((dev) >> 8) & 0xFF)
#define DEV_MINOR(dev)	((dev) & 0xFF)

struct blk_dev_ops {
	int (*d_rw)(struct buffer *bp, int rw);
};

int blk_devio(struct buffer *bp, int rw);

#endif /* DEV_H */
