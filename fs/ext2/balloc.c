/*
 * ext2/balloc.c
 * Block allocation routines for ext2.
 */
#include <asm/types.h>

#include <fs/ext2.h>
#include <fs/buffer.h>

#include <kernel/debug.h>
#include <kernel/mutex.h>

static ssize_t balloc_bgd(dev_t dev, struct ext2_superblock_m *sb, struct ext2_blk_group_desc *bgd);

ssize_t
ext2_balloc(dev_t dev, ino_t num)
{
	struct ext2_superblock_m *sb;
	struct ext2_blk_group_desc *bgd, *tmp;
	ssize_t block;

	/*
	 * First try to allocate a block from the inode's block group.
	 * If this fails, then try the other block groups.
	 */
	sb = get_ext2_superblock(dev);
	mutex_lock(&sb->mutex);
	if (sb->sb.s_free_blocks_count == 0) {
		block = 0;
		goto out;
	}
	bgd = sb->bgd_table + ((num - 1) % EXT2_INODES_PER_GROUP(sb));
	if ((block = balloc_bgd(dev, sb, bgd)) != 0)
		goto out;
	tmp = bgd;
	for (bgd = sb->bgd_table; bgd < sb->bgd_table + sb->nr_blk_group; bgd++) {
		if (bgd == tmp)
			continue;
		if ((block = balloc_bgd(dev, sb, bgd)) != 0)
			goto out;
	}
	block = 0;
 out:
	mutex_unlock(&sb->mutex);
	return block;
}

/*
 * Allocate a block from the given block group descriptor table.
 * Must be called with the superblock locked.
 */
static ssize_t
balloc_bgd(dev_t dev, struct ext2_superblock_m *sb, struct ext2_blk_group_desc *bgd)
{
	struct buffer *bp = NULL;
	char *p;
	ssize_t block;
	int i;

	if (bgd->bg_free_blocks_count == 0) {
		block = 0;
		goto out;
	}
	bp = bread(dev, bgd->bg_block_bitmap);
	for (p = bp->b_data, i = 0; *p & (1 << i); ) {
		if (p == bp->b_data + EXT2_BLOCKSIZE(sb)) {
			block = 0;
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
	block = ((p - bp->b_data) << 3) + i;
	/*
	 * Get actual logical block address. For each block group, blocks begin after the inode table;
	 * Actual block address is offset from the first block after the inode table.
	 */
	block += EXT2_INODES_PER_GROUP(sb) / EXT2_INODES_PER_BLOCK(sb) + bgd->bg_inode_table;
	bgd->bg_free_blocks_count--;
	sb->sb.s_free_blocks_count--;
	sb->modified = 1;
 out:
	if (bp != NULL)
		brelse(bp);
	return block;
}

void
ext2_bfree(dev_t dev, ino_t ino, ssize_t block)
{
	struct ext2_blk_group_desc *bgd;
	struct ext2_superblock_m *sb;
	struct buffer *bp;
	ssize_t b;
	char *p;
	int i;

	if (block == 0)
		return;
	sb = get_ext2_superblock(dev);
	mutex_lock(&sb->mutex);
	bgd = sb->bgd_table + ((ino - 1) % EXT2_INODES_PER_GROUP(sb));
	bp = bread(dev, bgd->bg_block_bitmap);
	b = block;
	b -= EXT2_INODES_PER_GROUP(sb) / EXT2_INODES_PER_BLOCK(sb) + bgd->bg_inode_table;
	p = bp->b_data;
	p += b >> 3;
	i = b % 8;
	if (!(*p & (1 << i)))
		panic("bfree: block is already free");
	*p &= ~(1 << i);
	bgd->bg_free_blocks_count++;
	sb->sb.s_free_blocks_count++;
	sb->modified = 1;
	bp->b_flags |= B_DWRITE;
	mutex_unlock(&sb->mutex);
	brelse(bp);
}
