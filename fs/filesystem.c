/*
 * filesystem.c
 * Generic filesystem code.
 */
#include <stddef.h>

#include <asm/types.h>

#include <fs/fs.h>

struct generic_filesystem *mount_table[NR_MOUNTS];
int nr_mounts = 0;

struct generic_filesystem *
find_filesystem(dev_t dev)
{
	struct generic_filesystem **fs;

	for (fs = mount_table; fs < mount_table + NR_MOUNTS; fs++) {
		if ((*fs)->f_dev == dev)
			return *fs;
	}
	return NULL;
}

struct generic_filesystem *
register_filesystem(struct generic_filesystem *fs)
{
	if (nr_mounts == NR_MOUNTS)
		return NULL;
	mount_table[nr_mounts++] = fs;
	return fs;
}

size_t
filesystem_get_attr(dev_t dev, enum fs_attribute_cmd cmd)
{
	struct generic_filesystem *fs;

	if ((fs = find_filesystem(dev)) == NULL)
		return 0;
	return fs->f_driver->fs_get_attribute(fs, cmd);
}
