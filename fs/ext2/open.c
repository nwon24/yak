/*
 * ext2/open.c
 * Handles the 'open' syscall for an ext2 file.
 * Also has a some other related syscalls.
 */
#include <fs/fs.h>
#include <fs/ext2.h>

#include <fs/dev.h>

#include <drivers/timer.h>

#include <generic/errno.h>
#include <generic/fcntl.h>
#include <generic/string.h>

#include <kernel/debug.h>
#include <kernel/proc.h>

static struct ext2_inode_m *ext2_new_file(const char *name, struct ext2_inode_m *dir, mode_t mode, dev_t dev, int *err);

/*
 * Pointer should already have been verified.
 */
void *
ext2_open(const char *path, int flags, int mode, int *err)
{
	struct ext2_inode_m *ip, *base_dir;
	const char *base;

	if ((ip = ext2_namei(path, err, &base, &base_dir, NULL)) == NULL) {
		if (!(flags & O_CREAT)) {
			*err = -ENOENT;
			return NULL;
		}
		if (!EXT2_S_ISDIR(base_dir->i_ino.i_mode))
			return NULL;
		if ((ip = ext2_new_file(base, base_dir, mode, 0, err)) != NULL)
			goto end;
		else
			return NULL;
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
 end:
	printk("ext2_open: mode %x ip->i_mode %x num %d %p\r\n", mode, ip->i_ino.i_mode, ip->i_num, ip);
	mutex_unlock(&ip->i_mutex);
	return ip;
}

/*
 * Creates a new file in the given directory with the given name.
 * If the mode specifies that it will be a special file, the device major/minor combo is also given.
 */
static struct ext2_inode_m *
ext2_new_file(const char *name, struct ext2_inode_m *dir, mode_t mode, dev_t dev, int *err)
{
	ino_t new;
	struct ext2_inode_m *ip;

	new = ext2_ialloc(dir->i_dev);
	if (new == 0) {
		*err = -ENOSPC;
		return NULL;
	}
	ip = ext2_iget(dir->i_dev, new);
	if (ip == NULL) {
		*err = -EIO;
		return NULL;
	}
	ip->i_ino.i_mode = mode;
	ip->i_ino.i_uid = current_process->euid;
	ip->i_ino.i_gid = current_process->egid;
	ip->i_ino.i_ctime = CURRENT_TIME;
	ip->i_ino.i_mtime = CURRENT_TIME;
	ip->i_ino.i_atime = CURRENT_TIME;
	ip->i_ino.i_dtime = 0;
	ip->i_ino.i_links_count = 1;
	ip->i_ino.i_blocks = 0;
	memset(ip->i_ino.i_block, 0, 15);
	if (EXT2_S_ISCHR(mode) || EXT2_S_ISBLK(mode))
		ip->i_ino.i_block[0] = dev;
	ip->i_flags |= I_MODIFIED;
	if ((*err = ext2_add_dir_entry(dir, ip, name, strlen(name))) < 0)
		return NULL;

	ext2_iput(ip);
	return ip;
}
