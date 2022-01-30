/*
 * fs/memdev.c
 * Device handling for character device number 1.
 */
#include <asm/uaccess.h>

#include <fs/dev.h>
#include <fs/fs.h>

#include <generic/errno.h>

static int memdev_null_rw(char *buf, int count, int rw);
static int memdev_zero_rw(char *buf, int count, int rw);

typedef int (*memdev_rw_fn)(char *buf, int count, int rw);

static memdev_rw_fn fn_table[] = {
	[MEMDEV_NULL] = memdev_null_rw,
	[MEMDEV_ZERO] = memdev_zero_rw,
};

static int
memdev_null_rw(char *buf, int count, int rw)
{
	return 0;
}

static int
memdev_zero_rw(char *buf, int count, int rw)
{
	int c;

	c = count;
	if (rw == READ) {
		while (c--) {
			put_ubyte(buf, 0);
			buf++;
		}
	}
	return c - count;
}

int
memdev_open(int minor)
{
	switch (minor) {
	case MEMDEV_NULL:
	case MEMDEV_ZERO:
		return 0;
	default:
		return -ENODEV;
	}
}

int
memdev_rw(int minor, char *buf, int count, int rw)
{
	switch (minor) {
	case MEMDEV_NULL:
	case MEMDEV_ZERO:
		return (*fn_table[minor])(buf, count, rw);
	default:
		return -ENODEV;
	}
}

int
memdev_close(int minor)
{
	return 0;
}
