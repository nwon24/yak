/*
 * sys1.c
 * Some file system system calls.
 */
#include <asm/uaccess.h>
#include <asm/paging.h>
#include <asm/types.h>

#include <fs/buffer.h>
#include <fs/fs.h>

#include <generic/errno.h>
#include <generic/fcntl.h>
#include <generic/stat.h>

#include <kernel/debug.h>
#include <kernel/proc.h>

static struct generic_filesystem *get_fs_from_path(const char *path);

struct file file_table[NR_FILE];

static inline struct file *
get_free_file(void)
{
	struct file *fp;

	for (fp = file_table; fp < file_table + NR_FILE; fp++) {
		if (fp->f_count == 0)
			return fp;
	}
	return NULL;
}

static inline int
find_fd(void)
{
	int i;

	for (i = 0; i < NR_FILE; i++) {
		if (current_process->file_table[i] == NULL)
			return i;
	}
	return -1;
}

/*
 * From a path, find the filesystem it is mounted under.
 * Returns NULL if invalid.
 * Pointer should already have been verified.
 */
static struct generic_filesystem *
get_fs_from_path(const char *path)
{
	if (path == NULL)
		return NULL;
	/*
	 * TODO; Do not support mounts yet.
	 * Just assume it is the root filesystem.
	 */
	return mount_table[0];
}

int
kernel_open(const char *path, int flags, mode_t mode)
{
	struct file *fp;
	struct generic_filesystem *fs;
	int fd, err;

	if (path == NULL || !check_user_ptr((void *)path))
		return -EINVAL;
	if ((fs = get_fs_from_path(path)) == NULL)
		return -EINVAL;
	if ((fd = find_fd()) < 0)
		return -ENFILE;
	if ((fp = get_free_file()) == NULL)
		return -ENFILE;
	if ((fs = get_fs_from_path(path)) == NULL)
		return -EINVAL;
	fp->f_fs = fs;
	fp->f_inode = fs->f_driver->fs_open(path, flags, mode, &err);
	if (fp->f_inode == NULL)
		return err;
	current_process->file_table[fd] = fp;
	fp->f_pos = 0;
	fp->f_mode = mode;
	fp->f_flags = flags;
	fp->f_count = 1;
	return fd;
}

int
kernel_creat(const char *path, mode_t mode)
{
	return kernel_open(path, O_WRONLY | O_CREAT | O_TRUNC, mode);
}

ssize_t
kernel_read(int fd, void *buf, size_t count)
{
	struct file *fp;

	if (buf == NULL || !check_user_ptr(buf))
		return -EFAULT;
	if (fd < 0)
		return -EBADF;
	fp = current_process->file_table[fd];
	if (fp == NULL || (fp->f_flags & O_ACCMODE) == O_WRONLY)
		return -EBADF;
	if (count == 0)
		return 0;
	return fp->f_fs->f_driver->fs_read(fp, buf, count);
}

ssize_t
kernel_write(int fd, void *buf, size_t count)
{
	struct file *fp;

	if (buf == NULL || !check_user_ptr(buf))
		return -EFAULT;
	if (fd < 0)
		return -EBADF;
	fp = current_process->file_table[fd];
	if (fp == NULL || (fp->f_flags & O_ACCMODE) == O_RDONLY)
		return -EBADF;
	if (count == 0)
		return 0;
	return fp->f_fs->f_driver->fs_write(fp, buf, count);
}

void
kernel_sync(void)
{
	struct generic_filesystem **fs;

	for (fs = mount_table; fs < mount_table + NR_MOUNTS; fs++) {
		if (*fs == NULL)
			continue;
		if ((*fs)->f_driver->fs_sync(*fs) < 0)
			printk("WARNING: Sync for device %x returned error.\r\n", (*fs)->f_dev);
	}
	buffer_sync();
}

int
kernel_unlink(const char *path)
{
	struct generic_filesystem *fs;

	if (path == NULL || !check_user_ptr((void *)path))
		return -EFAULT;
	fs = get_fs_from_path(path);
	if (fs == NULL)
		return -EINVAL;
	return fs->f_driver->fs_unlink(path);
}

off_t
kernel_lseek(int fd, off_t offset, int whence)
{
	struct file *fp;

	if (fd < 0)
		return -EBADF;
	fp = current_process->file_table[fd];
	if (fp == NULL)
		return -EBADF;
	return fp->f_fs->f_driver->fs_lseek(&fp->f_pos, fp->f_inode, offset, whence);
}

int
kernel_link(const char *path1, const char *path2)
{
	struct generic_filesystem *fs1, *fs2;

	if (path1 == NULL || path2 == NULL)
		return -EFAULT;
	if (!check_user_ptr((void *)path1) || !check_user_ptr((void *)path2))
		return -EFAULT;
	fs1 = get_fs_from_path(path1);
	fs2 = get_fs_from_path(path2);
	/*
	 * Don't support linking across different filesystems.
	 */
	if (fs1 != fs2)
		return -EXDEV;
	return fs1->f_driver->fs_link(path1, path2);
}

int
kernel_mknod(const char *path, mode_t mode, dev_t dev)
{
	struct generic_filesystem *fs;

	if (path == NULL || !check_user_ptr((void *)path))
		return -EFAULT;
	fs = get_fs_from_path(path);
	if (fs == NULL)
		return -EINVAL;
	/*
	 * Since 'mode' is made up of the POSIX bits check it here.
	 * It's likely the filesystem bits are the same anyway (such as ext2.
	 */
	switch (mode & S_IFMT) {
	case S_IFBLK:
	case S_IFCHR:
	case S_IFREG:
	case S_IFSOCK:
	case S_IFIFO:
		return fs->f_driver->fs_mknod(path, mode, dev);
	default:
		return -EINVAL;
	}
}

int
kernel_close(int fd)
{
	struct file **fp;

	if (fd >= NR_OPEN || fd < 0)
		return -EBADF;
	fp = current_process->file_table + fd;
	if (*fp == NULL)
		return -EBADF;
	if ((*fp)->f_count == 0)
		panic("kernel_close: f_count == 0");
	(*fp)->f_count--;
	(*fp)->f_fs->f_driver->fs_close(*fp);
	*fp = NULL;
	return 0;
}

int
kernel_chdir(const char *path)
{
	struct generic_filesystem *fs;

	if (path == NULL || !check_user_ptr((void *)path))
		return -EFAULT;
	fs = get_fs_from_path(path);
	if (fs == NULL)
		return -ENOENT;
	current_process->cwd_fs = fs;
	return fs->f_driver->fs_chdir(path);
}

int
kernel_chown(const char *name, uid_t uid, gid_t gid)
{
	struct generic_filesystem *fs;

	if (name == NULL || !check_user_ptr((void *)name))
		return -EFAULT;
	fs = get_fs_from_path(name);
	if (fs == NULL)
		return -EINVAL;
	if (fs->f_read_only)
		return -EROFS;
	return fs->f_driver->fs_chown(name, uid, gid);
}

int
kernel_chmod(const char *name, mode_t mode)
{
	struct generic_filesystem *fs;

	if (name == NULL || !check_user_ptr((void *)name))
		return -EFAULT;
	fs = get_fs_from_path(name);
	if (fs == NULL)
		return -EINVAL;
	if (fs->f_read_only)
		return -EROFS;
	return fs->f_driver->fs_chmod(name, mode);
}

int
kernel_fchmod(int fd, mode_t mode)
{
	struct file *fp;

	fp = current_process->file_table[fd];
	if (fp == NULL)
		return -EBADF;
	if (fp->f_fs->f_read_only)
		return -EROFS;
	return fp->f_fs->f_driver->fs_fchmod(fp, mode);
}

int
kernel_mkdir(const char *path, mode_t mode)
{
	struct generic_filesystem *fs;

	if (path == NULL || !check_user_ptr((void *)path))
		return -EFAULT;
	fs = get_fs_from_path(path);
	if (fs == NULL)
		return -EINVAL;
	if (fs->f_read_only)
		return -EROFS;
	return fs->f_driver->fs_mkdir(path, mode);
}

int
kernel_rmdir(const char *path)
{
	struct generic_filesystem *fs;

	if (path == NULL || !check_user_ptr((void *)path))
	    return -EFAULT;
	fs = get_fs_from_path(path);
	if (fs == NULL)
		return -EINVAL;
	if (fs->f_read_only)
		return -EROFS;
	return fs->f_driver->fs_rmdir(path);
}

int
kernel_chroot(const char *path)
{
	struct generic_filesystem *fs;

	if (path == NULL || !check_user_ptr((void *)path))
		return -EFAULT;
	fs = get_fs_from_path(path);
	if (fs == NULL)
		return -EINVAL;
	return fs->f_driver->fs_chroot(path);
}
