/*
 * ext2/super.c
 * Superblock related code for ext2.
 */
#include <fs/buffer.h>
#include <fs/dev.h>
#include <fs/ext2.h>
#include <fs/fs.h>

#include <kernel/debug.h>

#include <generic/string.h>

#include <mm/vm.h>

static struct fs_driver_ops ext2_driver_ops = {
	.fs_get_attribute = ext2_get_attribute
};

static struct generic_filesystem ext2_fs_struct;

int
ext2_init(void)
{
	struct buffer *bp;
	struct ext2_superblock_m *sb;

	/*
	 * Since the ext2 block size is in the superblock, just set
	 * the blocksize of the buffer to be DEFAULT_BLOCK_SIZE (1024),
	 * meaning the superblock is at logical block 1.
	 */
	if ((bp = getblk(CONFIG_FS_ROOT_DEV, 1)) == NULL)
		panic("ext2_init: getblk returned NULL");
	bp->b_blksize = DEFAULT_BLOCK_SIZE;
	bp->b_data = kvmalloc(DEFAULT_BLOCK_SIZE);
	blk_devio(bp, READ);
	if (bp->b_flags & B_ERROR)
		panic("ext2_init: unable to read superblock");
	ext2_fs_struct.f_fs = EXT2;
	ext2_fs_struct.f_dev = CONFIG_FS_ROOT_DEV;
	ext2_fs_struct.f_driver = &ext2_driver_ops;
	ext2_fs_struct.f_super = kvmalloc(sizeof(struct ext2_superblock_m));
	memmove(ext2_fs_struct.f_super, bp->b_data, sizeof(struct ext2_superblock_m));
	brelse(bp);
	if (register_filesystem(&ext2_fs_struct) == NULL)
		panic("ext2_init: unable to register filesystem");
	sb = ext2_fs_struct.f_super;
	if (sb->sb.s_magic != EXT2_MAGIC)
		panic("ext2_init: Root device not an ext2 filesystem");
	/* TODO: Mount root */
	return 0;
}

/*
 * We have to make sure this never fails as we use
 * macros that depend on it.
 */
struct ext2_superblock *
get_ext2_superblock(struct generic_filesystem *fs)
{
	if (fs->f_fs != EXT2)
		panic("get_ext2_superblock: mount table corrupted");
	return (struct ext2_superblock *)fs->f_super;
}
