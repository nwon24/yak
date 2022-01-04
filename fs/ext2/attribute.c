/*
 * ext2/attribute.c
 * Not much to see here.
 */
#include <asm/types.h>

#include <fs/fs.h>
#include <fs/ext2.h>

size_t
ext2_get_attribute(struct generic_filesystem *fs, enum fs_attribute_cmd cmd)
{
	switch (cmd) {
	case GET_BLOCKSIZE:
		return EXT2_BLOCKSIZE(fs);
		break;
	case GET_MTIME:
		return EXT2_MTIME(fs);
		break;
	case GET_WTIME:
		return EXT2_WTIME(fs);
		break;
	}
	return 0;
}
