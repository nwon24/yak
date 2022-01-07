/*
 * ext2/super.c
 * Superblock related code for ext2.
 */
#include <fs/buffer.h>
#include <fs/dev.h>
#include <fs/ext2.h>
#include <fs/fs.h>

#include <kernel/debug.h>
#include <kernel/proc.h>

#include <generic/string.h>

#include <mm/vm.h>

static struct fs_driver_ops ext2_driver_ops = {
	.fs_get_attribute = ext2_get_attribute
};

/*
 * Statically allocated for root filesystem.
 * Mounts should be dynamically allocated since they can
 * be unmounted during the system's life. Root is only
 * unmounted at shutdown.
 */
static struct generic_filesystem ext2_fs_struct;

static void ext2_mount_root(void);

int
ext2_init(void)
{
	struct buffer *bp;
	struct ext2_superblock_m *sb;
	int tmp1, tmp2;

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
	if (register_filesystem(&ext2_fs_struct) == NULL)
		panic("ext2_init: unable to register filesystem");
	sb = ext2_fs_struct.f_super;
	if (sb->sb.s_magic != EXT2_MAGIC)
		panic("ext2_init: Root device not an ext2 filesystem");
	if ((sb->sb.s_inodes_count % sb->sb.s_inodes_per_group) == 0)
		tmp1 = sb->sb.s_inodes_count / sb->sb.s_inodes_per_group;
	else
		tmp1 = sb->sb.s_inodes_count / sb->sb.s_inodes_per_group + 1;
	if ((sb->sb.s_blocks_count % sb->sb.s_blocks_per_group) == 0)
		tmp2 = sb->sb.s_blocks_count / sb->sb.s_blocks_per_group;
	else
		tmp2 = sb->sb.s_blocks_count / sb->sb.s_blocks_per_group + 1;
	if (tmp1 != tmp2)
		panic("ext2_init: Superblock corrupted");
	sb->nr_blk_group = tmp1;
	if ((sb->bgd_table = kvmalloc(tmp1 * sizeof(*sb->bgd_table))) == NULL)
		panic("ext2_init: Unable to allocate memory for block group descriptor table");
	bp->b_blksize = 1024 << sb->sb.s_log_block_size;
	if (bp->b_blksize == 1024)
		bp->b_blknr = 2;
	else
		bp->b_blknr = 1;
	blk_devio(bp, READ);
	memmove(sb->bgd_table, bp->b_data, tmp1 * sizeof(*sb->bgd_table));
	ext2_inodes_init();
	register_mount_root_routine(ext2_mount_root);
	return 0;
}

/*
 * We have to make sure this never fails as we use
 * macros that depend on it.
 */
struct ext2_superblock_m *
get_ext2_superblock(dev_t dev)
{
	struct generic_filesystem *fs;

	if ((fs = find_filesystem(dev)) == NULL)
		panic("get_ext2_superblock: invalid device");
	if (fs->f_fs != EXT2)
		panic("get_ext2_superblock: mount table corrupted");
	return (struct ext2_superblock_m *)fs->f_super;
}

static void
ext2_mount_root(void)
{
	struct ext2_inode_m *ip;

	ip = ext2_iget(CONFIG_FS_ROOT_DEV, EXT2_ROOT_INO);
	current_process->root_inode = ip;
	if (current_process->root_inode == NULL)
		panic("ext2_mount_root: unable to get root inode");
	current_process->cwd_inode = current_process->root_inode;
}
