/*
 * ext2/read_write.c
 * Implements the 'read' and 'write' system calls for ext2.
 */
#include <fs/dev.h>
#include <fs/fs.h>
#include <fs/ext2.h>

#include <generic/string.h>
#include <generic/errno.h>

#include <kernel/debug.h>

int
ext2_read(struct file *file, void *buf, size_t count)
{
	struct ext2_inode_m *ip;

	if (file->f_fs->f_fs != EXT2)
		return -EINVAL;
	ip = file->f_inode;
	if (ip == NULL) {
		printk("ext2_read: file has no inode!\r\n");
		return -EINVAL;
	}
	if (EXT2_S_ISDIR(ip->i_ino.i_mode) || EXT2_S_ISREG(ip->i_ino.i_mode))
		panic("TODO: Implement file read for ext2");
	if (EXT2_S_ISBLK(ip->i_ino.i_mode))
		panic("TODO: Implement block read for ext2");
	if (EXT2_S_ISCHR(ip->i_ino.i_mode))
		return chr_devio(ip->i_ino.i_block[0], buf, count, READ);
	return -EINVAL;
}

int
ext2_write(struct file *file, void *buf, size_t count)
{
	struct ext2_inode_m *ip;

	if (file->f_fs->f_fs != EXT2)
		return -EINVAL;
	ip = file->f_inode;
	if (ip == NULL) {
		printk("ext2_write: file has no inode!\r\n");
		return -EINVAL;
	}
	if (EXT2_S_ISDIR(ip->i_ino.i_mode) || EXT2_S_ISREG(ip->i_ino.i_mode))
		panic("TODO: Implement file write for ext2");
	if (EXT2_S_ISBLK(ip->i_ino.i_mode))
		panic("TODO: Implement block write for ext2");
	if (EXT2_S_ISCHR(ip->i_ino.i_mode))
		return chr_devio(ip->i_ino.i_block[0], buf, count, WRITE);
	return -EINVAL;
}
