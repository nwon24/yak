/*
 * ext2/open.c
 * Handles the 'open' syscall for an ext2 file.
 */
#include <fs/fs.h>
#include <fs/ext2.h>

#include <fs/dev.h>

#include <generic/errno.h>
#include <generic/fcntl.h>

#include <kernel/debug.h>
#include <kernel/proc.h>

/*
 * Pointer should already have been verified.
 */
void *
ext2_open(const char *path, int flags, int mode, int *err)
{
	struct ext2_inode_m *ip;

	if ((ip = ext2_namei(path, err)) == NULL) {
		if (!(flags & O_CREAT))
			return NULL;
		panic("TODO: Implement creation of files using open()");
	}
	mode &= ~current_process->umask & 0777;
	if (!((flags & O_ACCMODE) == O_WRONLY) && ext2_permission(ip, PERM_READ) < 0) {
		*err = -EACCES;
		return NULL;
	}
	if (!((flags & O_ACCMODE) == O_RDONLY) && ext2_permission(ip, PERM_WRITE) < 0) {
		*err = -EACCES;
		return NULL;
	}

	if (!((flags & O_ACCMODE) == O_RDONLY) && EXT2_S_ISDIR(ip->i_ino.i_mode)) {
		*err = -EISDIR;
		return NULL;
	}

	if ((flags & O_TRUNC) && EXT2_S_ISREG(ip->i_ino.i_mode))
		ext2_itrunc(ip);

	if (EXT2_S_ISCHR(ip->i_ino.i_mode))
		chr_dev_open(ip->i_ino.i_block[0]);
	printk("ext2_open: ip->i_mode %x\r\n", ip->i_ino.i_mode);
	mutex_unlock(&ip->i_mutex);
	return ip;
}
