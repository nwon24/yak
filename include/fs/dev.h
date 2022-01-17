#ifndef DEV_H
#define DEV_H

#include <drivers/tty.h>

#include <fs/buffer.h>

#define NODEV	0

/* Block device numbers */
#define RD_DEV	1
#define FD_DEV	2
#define HD_DEV	3
/* Character device numbers */
#define TTY_DEV		4
#define TTYX_DEV	5

#define DEV_MAJOR(dev)	(((dev) >> 8) & 0xFF)
#define DEV_MINOR(dev)	((dev) & 0xFF)

static inline int
is_chardev(dev_t dev)
{
	switch DEV_MAJOR(dev) {
	case TTY_DEV:
	case TTYX_DEV:
		return 1;
	default:
		return 0;
	}
}

struct blk_dev_ops {
	int (*d_rw)(struct buffer *bp, int rw);
};

struct chr_dev_ops {
	int (*d_open)(int minor);
	int (*d_rw)(int minor, char *buf, int count, int rw);
	int (*d_close)(int minor);
};

int blk_devio(struct buffer *bp, int rw);

#endif /* DEV_H */
