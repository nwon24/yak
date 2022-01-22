/*
 * ext2/open.c
 * Handles the 'open' syscall for an ext2 file.
 * Also has a some other related syscalls.
 */
#include <asm/uaccess.h>

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
	const char *base, *p;
	void *ret;

	if ((ip = ext2_namei(path, err, &base, &base_dir, NULL)) == NULL) {
		if (!(flags & O_CREAT)) {
			ext2_iput(base_dir);
			*err = -ENOENT;
			ret = NULL;
			goto end;
		}
		if (!EXT2_S_ISDIR(base_dir->i_ino.i_mode)) {
			*err = -ENOTDIR;
			ret = NULL;
			goto end;
		}
		for (p = base; get_ubyte(p) != '\0'; p++) {
			if (get_ubyte(p) == '/') {
				*err = -ENOENT;
				ret = NULL;
				goto end;
			}
		}
		if ((ip = ext2_new_file(base, base_dir, mode, 0, err)) != NULL) {
			ret = ip;
			goto end;
		} else {
			ret = NULL;
			goto end;
		}
	}
	mode &= ~current_process->umask & 0777;
	if (!((flags & O_ACCMODE) == O_WRONLY) && ext2_permission(ip, PERM_READ) < 0) {
		*err = -EACCES;
		ret = NULL;
		goto end;
	}
	if (!((flags & O_ACCMODE) == O_RDONLY) && ext2_permission(ip, PERM_WRITE) < 0) {
		*err = -EACCES;
		ret = NULL;
		goto end;
	}

	if (!((flags & O_ACCMODE) == O_RDONLY) && EXT2_S_ISDIR(ip->i_ino.i_mode)) {
		*err = -EISDIR;
		ret = NULL;
		goto end;
	}

	if ((flags & O_TRUNC) && EXT2_S_ISREG(ip->i_ino.i_mode))
		ext2_itrunc(ip);

	if (EXT2_S_ISCHR(ip->i_ino.i_mode))
		chr_dev_open(ip->i_ino.i_block[0]);
	ret = ip;
 end:
	printk("ext2_open: mode %x ip->i_mode %x num %d %p\r\n", mode, ip->i_ino.i_mode, ip->i_num, ip);
	if (base_dir != ip)
		ext2_iput(base_dir);
	mutex_unlock(&ip->i_mutex);
	return ret;
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
	struct ext2_superblock_m *sb = get_ext2_superblock(dir->i_dev);

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
	memset(&ip->i_ino, 0, EXT2_INODE_SIZE(sb));
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
	if ((*err = ext2_add_dir_entry(dir, ip, name, strlen(name))) < 0) {
		ext2_iput(ip);
		return NULL;
	}
	return ip;
}

int
ext2_mknod(const char *path, mode_t mode, dev_t dev)
{
	struct ext2_inode_m *dp, *ip;
	const char *base, *p;
	int err;

	mode &= ~current_process->umask;
	ip = ext2_namei(path, &err, &base, &dp, NULL);
	/*
	 * We need to check no entry was found.
	 * If it didn't find a directory, thatn -ENOENT should be returned.
	 * If it didn't find the final componenet in the path, then that is good, since
	 * that is what we are trying to make.
	 */
	if (ip != NULL) {
		ext2_iput(ip);
		if (ip != dp)
			ext2_iput(dp);
		return -EEXIST;
	}
	/*
	 * If it failed for some other reason, just return that reason.
	 */
	if (err != -ENOENT) {
		ext2_iput(dp);
		return err;
	}
	/*
	 * So now, we know that 'ext2_namei' failed by not being able to find an entry.
	 * Just need to determine where it was a directory componenet or the final component (which shouldn't exist).
	 */
	p = base;
	while (get_ubyte(p) != '\0' && get_ubyte(p) != '/')
		p++;
	if (get_ubyte(p) == '/') {
		ext2_iput(dp);
		return -ENOENT;
	}
	if ((ip = ext2_new_file(base, dp, mode, dev, &err)) == NULL) {
		ext2_iput(dp);
		return err;
	}
	ext2_iput(ip);
	ext2_iput(dp);
	return 0;
}

/*
 * Reference count has already been dealt with, as that is not filesystem
 * dependent.
 */
int
ext2_close(struct file *file)
{
	if (file == NULL || file->f_inode == NULL)
		panic("ext2_close: file == NULL or file->f_inode == NULL");
	if (file->f_count == 0)
		ext2_iput(file->f_inode);
	return 0;
}
