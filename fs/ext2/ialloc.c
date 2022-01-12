/*
 * ext2/ialloc.c
 * Inode allocation routines for ext2.
 */
#include <asm/types.h>

#include <fs/ext2.h>
#include <fs/buffer.h>

#include <kernel/debug.h>
#include <kernel/mutex.h>

static ino_t ialloc_bgd(dev_t dev, struct ext2_superblock_m *sb, struct ext2_blk_group_desc *bgd);

ino_t
ext2_ialloc(dev_t dev)
{
	struct ext2_superblock_m *sb;
	struct ext2_blk_group_desc *bgd;
	ino_t num = 0;

	/*
	 * Much simpler than allocating a block as we just iterate over the block groups to find
	 * the first group with a free inode.
	 */
	sb = get_ext2_superblock(dev);
	mutex_lock(&sb->mutex);
	if (sb->sb.s_free_inodes_count == 0)
		goto out;

	for (bgd = sb->bgd_table; bgd < sb->bgd_table + sb->nr_blk_group; bgd++) {
		if ((num = ialloc_bgd(dev, sb, bgd)) != 0)
			goto out;
	}
 out:
	mutex_unlock(&sb->mutex);
	return num;
}

/*
 * Allocate an inode from the block group specified by the block group descriptor.
 */
static ino_t
ialloc_bgd(dev_t dev, struct ext2_superblock_m *sb, struct ext2_blk_group_desc *bgd)
{
	struct buffer *bp = NULL;
	char *p;
	ino_t num;
	int i;

	if (bgd->bg_free_inodes_count == 0) {
		num = 0;
		goto out;
	}
	bp = bread(dev, bgd->bg_inode_bitmap);
	for (p = bp->b_data, i = 0; *p & (1 << i); ) {
		if (p == bp->b_data + EXT2_BLOCKSIZE(sb)) {
			num = 0;
			goto out;
		}
		if (i == 7) {
			i = 0;
			p++;
		} else {
			i++;
		}
	}
	*p |= 1 << i;
	bp->b_flags |= B_DWRITE;
	/*
	 * Inode numbers begin at 1, in contrast to block addresses.
	 */
	num = ((p - bp->b_data) << 3) + i + 1;
	bgd->bg_free_inodes_count--;
	sb->sb.s_free_inodes_count--;
	sb->modified = 1;
 out:
	if (bp != NULL)
		brelse(bp);
	return num;
}

void
ext2_ifree(dev_t dev, ino_t ino)
{
	struct ext2_blk_group_desc *bgd;
	struct ext2_superblock_m *sb;
	struct buffer *bp = NULL;
	char *p;
	int i;

	sb = get_ext2_superblock(dev);
	mutex_lock(&sb->mutex);
	/*
	 * Do nothing if the inode is one of the reserved ones.
	 * Reserved inodes are specified in the superblock if the version is more than 1,
	 * otherwise it is 11.
	 */
	if (ino < EXT2_FIRST_FREE_INO(sb))
		goto out;
	bgd = sb->bgd_table + ((ino - 1) % EXT2_INODES_PER_GROUP(sb));
	bp = bread(dev, bgd->bg_inode_bitmap);
	p = bp->b_data;
	ino--;
	p += ino >> 3;
	i = ino % 8;
	if (!(*p & (1 << i)))
		panic("bfree: block is already free");
	*p &= ~(1 << i);
	bgd->bg_free_inodes_count++;
	sb->sb.s_free_inodes_count++;
	sb->modified = 1;
	bp->b_flags |= B_DWRITE;
 out:
	mutex_unlock(&sb->mutex);
	if (bp != NULL)
		brelse(bp);
}
