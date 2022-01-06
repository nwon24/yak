/*
 * ext2/attribute.c
 * Not much to see here.
 */
#include <asm/types.h>

#include <fs/fs.h>
#include <fs/ext2.h>

#include <kernel/debug.h>

size_t
ext2_get_attribute(struct generic_filesystem *fs, enum fs_attribute_cmd cmd)
{
	struct ext2_superblock_m *sb;

	if (fs->f_fs != EXT2)
		return 0;
	sb = (struct ext2_superblock_m *)fs->f_super;
	if (sb == NULL)
		panic("ext2 filesystem registered with no superblock");
	switch (cmd) {
	case GET_BLOCKSIZE:
		return EXT2_BLOCKSIZE(sb);
		break;
	case GET_MTIME:
		return EXT2_MTIME(sb);
		break;
	case GET_WTIME:
		return EXT2_WTIME(sb);
		break;
	}
	return 0;
}
