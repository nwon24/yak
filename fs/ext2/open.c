/*
 * ext2/open.c
 * Handles the 'open' syscall for an ext2 file.
 * Also has a some other syscalls that don't seem to be belong in their own file.
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

static int _ext2_chmod(struct ext2_inode_m *ip, mode_t mode);
static int _ext2_chown(struct ext2_inode_m *ip, uid_t uid, gid_t gid);

/*
 * Pointer should already have been verified.
 */
void *
ext2_open(const char *path, int flags, int mode, int *err)
{
	struct ext2_inode_m *ip, *base_dir;
	const char *base, *p;
	void *ret;

	if ((ip = ext2_namei(path, err, &base, &base_dir, NULL, NULL, NULL)) == NULL) {
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
struct ext2_inode_m *
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
		ip->i_ino.i_links_count = 0;
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
	ip = ext2_namei(path, &err, &base, &dp, NULL, NULL, NULL);
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

int
ext2_lchown(const char *path, uid_t uid, gid_t gid)
{
	struct ext2_inode_m *ip;
	int err;

	ip = ext2_namei(path, &err, NULL, NULL, NULL, NULL, NULL);
	if (ip == NULL)
		return err;
	return _ext2_chown(ip, uid, gid);
}

int
ext2_chown(const char *path, uid_t uid, gid_t gid)
{
	struct ext2_inode_m *ip, *dp, *target;
	int err;

	ip = ext2_namei(path, &err, NULL, &dp, NULL, NULL, NULL);
	if (ip == NULL)
		return err;
	if (EXT2_S_ISLNK(ip->i_ino.i_mode)) {
		target = ext2_follow_symlink(ip, current_process->root_inode, dp, &err);
		if (target == NULL) {
			ext2_iput(ip);
			if (dp != ip)
				ext2_iput(dp);
			return err;
		}
		ext2_iput(ip);
	} else {
		target = ip;
	}
	if (dp != ip)
		ext2_iput(dp);
	return _ext2_chown(target, uid, gid);
}

static int
_ext2_chown(struct ext2_inode_m *ip, uid_t uid, gid_t gid)
{
	if (current_process->euid != ip->i_ino.i_uid) {
		ext2_iput(ip);
		return -EPERM;
	}
	if (uid != -1) {
		ip->i_ino.i_uid = uid;
		ip->i_flags |= I_MODIFIED;
	}
	if (gid != -1) {
		ip->i_ino.i_gid = gid;
		ip->i_flags |= I_MODIFIED;
	}
	ext2_iput(ip);
	return 0;
}

static int
_ext2_chmod(struct ext2_inode_m *ip, mode_t mode)
{
	if (ip->i_ino.i_gid != current_process->egid)
		mode &= ~EXT2_S_ISGID;
	mode &= 07777;
	if (current_process->euid == ip->i_ino.i_uid) {
		ip->i_ino.i_mode = (ip->i_ino.i_mode & EXT2_S_IFMT) | mode;
	} else {
		ext2_iput(ip);
		return -EACCES;
	}
	ip->i_ino.i_mtime = CURRENT_TIME;
	ip->i_flags |= I_MODIFIED;
	return 0;
}

int
ext2_chmod(const char *path, mode_t mode)
{
	struct ext2_inode_m *ip;
	int err;

	ip = ext2_namei(path, &err, NULL, NULL, NULL, NULL, NULL);
	if (ip == NULL)
		return err;
	err = _ext2_chmod(ip, mode);
	ext2_iput(ip);
	return err;
}

int
ext2_fchmod(struct file *file, mode_t mode)
{
	return _ext2_chmod(file->f_inode, mode);
}

int
ext2_chroot(const char *path)
{
	struct ext2_inode_m *ip, *dp;
	const char *p, *tmp;
	int err;

	ip = ext2_namei(path, &err, &p, &dp, NULL, NULL, NULL);
	if (ip == NULL) {
		if (err != ENOENT) {
			ext2_iput(dp);
			return err;
		}
		for (tmp = p; get_ubyte(tmp) != '\0' && get_ubyte(tmp) != '/'; p++);
		if (get_ubyte(tmp) == '\0') {
			ext2_iput(dp);
			return -ENOENT;
		} else {
			if (get_ubyte(tmp + 1) != '\0') {
				ext2_iput(dp);
				return -ENOENT;
			}
			ip = dp;
		}
	}
	if (!EXT2_S_ISDIR(ip->i_ino.i_mode)) {
		ext2_iput(ip);
		return -ENOTDIR;
	}
	ext2_iput(current_process->root_inode);
	current_process->root_inode = ip;
	return 0;
}
