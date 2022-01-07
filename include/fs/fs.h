#ifndef FS_H
#define FS_H

#include <stddef.h>

#include <asm/types.h>

#include <kernel/mutex.h>

#define DEFAULT_BLOCK_SIZE	1024

#define NR_MOUNTS	10

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
struct fs_driver_ops {
	size_t (*fs_get_attribute)(struct generic_filesystem *fs, enum fs_attribute_cmd cmd);
};

struct generic_filesystem {
	enum filesystem f_fs;
	void *f_super;
	dev_t f_dev;
	mutex f_mutex;
	struct fs_driver_ops *f_driver;
};

struct file {
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

#endif /* FS_H */
