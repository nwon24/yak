/*
 * ext2/open.c
 * Handles the 'open' syscall for an ext2 file.
 */
#include <fs/fs.h>
#include <fs/ext2.h>

#include <generic/errno.h>

#include <kernel/debug.h>

/*
 * Pointer should already have been verified.
 */
void *
ext2_open(const char *path, int flags, int mode, int *err)
{
	struct ext2_inode_m *ip;
	int rand;

	if ((ip = ext2_namei(path, err)) == NULL)
		return NULL;

	rand = flags;
	rand = mode;
	rand++;
	printk("open %p\r\n", ip);
	printk("ext2_open: ip->i_mode %x\r\n", ip->i_ino.i_mode);
	return ip;
}
