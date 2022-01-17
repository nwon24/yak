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

ssize_t
kernel_read(int fd, void *buf, size_t count)
{
	struct file *fp;

	if (buf == NULL || !check_user_ptr(buf))
		return -EFAULT;
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
