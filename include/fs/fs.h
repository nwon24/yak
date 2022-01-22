#ifndef FS_H
#define FS_H

#include <stddef.h>

#include <asm/types.h>
#include <asm/syscall.h>

#include <fs/buffer.h>

#include <generic/unistd.h>

#include <kernel/mutex.h>

#define DEFAULT_BLOCK_SIZE	1024

#define NR_MOUNTS	10

#define NR_FILE		64
#define NR_OPEN		20

enum {
	READ,
	WRITE
};

enum filesystem {
	EXT2
	/* We don't support any other filesystems... */
};

enum fs_attribute_cmd {
	GET_BLOCKSIZE,
	GET_MTIME,
	GET_WTIME,
};

struct generic_filesystem;
/*
 * Will add to this.
 */
struct file;

struct fs_driver_ops {
	size_t (*fs_get_attribute)(struct generic_filesystem *fs, enum fs_attribute_cmd cmd);
	void *(*fs_open)(const char *path, int flags, int mode, int *err);
	int (*fs_read)(struct file *file, void *buf, size_t count);
	int (*fs_write)(struct file *file, void *buf, size_t count);
	int (*fs_sync)(struct generic_filesystem *fs);
	int (*fs_unlink)(const char *path);
	off_t (*fs_lseek)(off_t *ptr, void *inode, off_t offset, int whence);
	int (*fs_link)(const char *path1, const char *path2);
	int (*fs_mknod)(const char *path, mode_t mode, dev_t dev);
	int (*fs_close)(struct file *file);
	int (*fs_chdir)(const char *path);
	int (*fs_chown)(const char *path, uid_t uid, gid_t gid);
};

struct generic_filesystem {
	enum filesystem f_fs;
	void *f_super;
	dev_t f_dev;
	mutex f_mutex;
	struct fs_driver_ops *f_driver;
};

extern struct generic_filesystem *mount_table[];

struct file {
	struct generic_filesystem *f_fs;
	void *f_inode;	/* Filesystem inode */
	off_t f_pos;	/* Position in file */
	int f_mode;
	int f_flags;
	unsigned int f_count;
};

struct generic_filesystem *find_filesystem(dev_t dev);
struct generic_filesystem *register_filesystem(struct generic_filesystem *fs);
size_t filesystem_get_attr(dev_t dev, enum fs_attribute_cmd cmd);
void register_mount_root_routine(void (*routine)(void));

extern void (*do_mount_root)(void);

int kernel_open(const char *path, int flags, int mode);
ssize_t kernel_read(int fd, void *buf, size_t count);
ssize_t kernel_write(int fd, void *buf, size_t count);
void kernel_sync(void);
int kernel_unlink(const char *path);
off_t kernel_lseek(int fd, off_t offset, int whence);
int kernel_creat(const char *path, mode_t mode);
int kernel_link(const char *path1, const char *path2);
int kernel_mknod(const char *path, mode_t mode, dev_t dev);
int kernel_close(int fd);
int kernel_chdir(const char *path);
int kernel_chown(const char *path, uid_t uid, gid_t gid);

static inline void
fs_init(void)
{
	buffer_init();
	register_syscall(__NR_open, (uint32_t)kernel_open, 3);
	register_syscall(__NR_read, (uint32_t)kernel_read, 3);
	register_syscall(__NR_write, (uint32_t)kernel_write, 3);
	register_syscall(__NR_sync, (uint32_t)kernel_sync, 0);
	register_syscall(__NR_unlink, (uint32_t)kernel_unlink, 1);
	register_syscall(__NR_lseek, (uint32_t)kernel_lseek, 3);
	register_syscall(__NR_creat, (uint32_t)kernel_creat, 2);
	register_syscall(__NR_link, (uint32_t)kernel_link, 2);
	register_syscall(__NR_mknod, (uint32_t)kernel_mknod, 3);
	register_syscall(__NR_close, (uint32_t)kernel_close, 1);
	register_syscall(__NR_chdir, (uint32_t)kernel_chdir, 1);
	register_syscall(__NR_chown, (uint32_t)kernel_chown, 3);
}

#endif /* FS_H */
