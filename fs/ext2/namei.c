/*
 * ext2/namei.c
 * Converts a path name into an inode.
 */
#include <fs/ext2.h>
#include <fs/fs.h>

#include <kernel/proc.h>
#include <kernel/debug.h>

struct ext2_inode_m *
ext2_namei(const char *path)
{
	if (check_user_ptr(path) == 0)
		panic("Invalid pointer passed");
	return NULL;
}
