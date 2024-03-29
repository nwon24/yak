/*
 * ext2/balloc.c
 * Block allocation and free routines for ext2.
 */
#include <asm/types.h>

#include <fs/ext2.h>
#include <fs/buffer.h>

#include <drivers/drive.h>
#include <drivers/timer.h>

#include <generic/string.h>

#include <kernel/debug.h>
#include <kernel/mutex.h>

static ext2_block balloc_bgd(dev_t dev, struct ext2_superblock_m *sb, struct ext2_blk_group_desc *bgd);

ext2_block
ext2_balloc(struct ext2_inode_m *ip)
{
	struct ext2_superblock_m *sb;
	struct ext2_blk_group_desc *bgd, *tmp;
	struct buffer *bp;
	ext2_block block;

	/*
	 * First try to allocate a block from the inode's block group.
	 * If this fails, then try the other block groups.
	 */
	sb = get_ext2_superblock(ip->i_dev);
	if (sb->sb.s_free_blocks_count == 0) {
		block = 0;
		goto out;
	}
	bgd = sb->bgd_table + ((ip->i_num - 1) / EXT2_INODES_PER_GROUP(sb));
	if ((block = balloc_bgd(ip->i_dev, sb, bgd)) != 0)
		goto out;
	tmp = bgd;
	for (bgd = sb->bgd_table; bgd < sb->bgd_table + sb->nr_blk_group; bgd++) {
		if (bgd == tmp)
			continue;
		if ((block = balloc_bgd(ip->i_dev, sb, bgd)) != 0)
			goto out;
	}
	block = 0;
 out:
	bp = getblk(ip->i_dev, block);
	memset(bp->b_data, 0, EXT2_BLOCKSIZE(sb));
	ip->i_ino.i_blocks += EXT2_BLOCKSIZE(sb) / SECTOR_SIZE;
	ip->i_ino.i_mtime = CURRENT_TIME;
	ip->i_flags |= I_MODIFIED;
	bp->b_flags |= B_DWRITE;
	brelse(bp);
	return block;
}

/*
 * Allocate a block from the given block group descriptor table.
 * Superblock is not locked - we lock it for least amount of time possible
 * and without calling any other functions.
 */
static ext2_block
balloc_bgd(dev_t dev, struct ext2_superblock_m *sb, struct ext2_blk_group_desc *bgd)
{
	struct buffer *bp = NULL;
	ext2_block block;
	int off;
	uint8_t *p;

	if (bgd->bg_free_blocks_count == 0) {
		block = 0;
		goto out;
	}

	bp = bread(dev, bgd->bg_block_bitmap);
	for (p = (uint8_t *)bp->b_data, off = 0; p < (uint8_t *)bp->b_data + EXT2_BLOCKSIZE(sb); ) {
		if (!(*p & (1 << (off % 8))))
			break;
		off++;
		if (!(off % 8))
			p++;
	}
	*p |= 1 << (off % 8);
	bp->b_flags |= B_DWRITE;
	block = off;
	/*
	 * Get actual block address by adding the offset to the number of blocks in all
	 * the previous block groups and the first data block.
	 * This is where some issues have been caused in the past.
	 */
	block += (bgd - sb->bgd_table) * EXT2_BLOCKS_PER_GROUP(sb) + sb->sb.s_first_data_block;
	mutex_lock(&sb->mutex);
	bgd->bg_free_blocks_count--;
	sb->sb.s_free_blocks_count--;
	sb->modified = 1;
	mutex_unlock(&sb->mutex);
 out:
	if (bp != NULL)
		brelse(bp);
	return block;
}

/*
 * Don't rely on inode number as an inode may be assigned a block outside of its block group,
 * if there were no free blocks in its block group at the time of allocation.
 */
void
ext2_bfree(dev_t dev, ext2_block block)
{
	struct ext2_blk_group_desc *bgd;
	struct ext2_superblock_m *sb;
	struct buffer *bp;
	ext2_block b;
	char *p;
	int i;

	if (block == 0)
		return;
	sb = get_ext2_superblock(dev);
	bgd = sb->bgd_table + (block / EXT2_BLOCKS_PER_GROUP(sb));
	bp = bread(dev, bgd->bg_block_bitmap);
	b = block;
	b -= sb->sb.s_first_data_block + ((bgd - sb->bgd_table) * EXT2_BLOCKS_PER_GROUP(sb));
	p = bp->b_data;
	p += b >> 3;
	i = b % 8;
	if (!(*p & (1 << i)))
		panic("bfree: block is already free");
	*p &= ~(1 << i);
	mutex_lock(&sb->mutex);
	bgd->bg_free_blocks_count++;
	sb->sb.s_free_blocks_count++;
	sb->modified = 1;
	mutex_unlock(&sb->mutex);
	bp->b_flags |= B_DWRITE;
	brelse(bp);
}

void
ext2_bfree_indirect(dev_t dev, ext2_block block)
{
	struct buffer *bp;
	struct ext2_superblock_m *sb;
	uint32_t *b;

	if (block == 0)
		return;
	sb = get_ext2_superblock(dev);
	bp = bread(dev, block);
	for (b = (uint32_t *)bp->b_data; b < (uint32_t *)bp->b_data + (EXT2_BLOCKSIZE(sb) / sizeof(uint32_t)); b++)
		ext2_bfree(dev, *b);
	ext2_bfree(dev, block);
	brelse(bp);
}

void
ext2_bfree_dindirect(dev_t dev, ext2_block block)
{
	struct buffer *bp;
	struct ext2_superblock_m *sb;
	uint32_t *b;

	if (block == 0)
		return;
	sb = get_ext2_superblock(dev);
	bp = bread(dev, block);
	for (b = (uint32_t *)bp->b_data; b < (uint32_t *)bp->b_data + (EXT2_BLOCKSIZE(sb) / sizeof(uint32_t)); b++)
		ext2_bfree_indirect(dev, *b);
	ext2_bfree(dev, block);
	brelse(bp);
}

void
ext2_bfree_tindirect(dev_t dev, ext2_block block)
{
	struct buffer *bp;
	struct ext2_superblock_m *sb;
	uint32_t *b;

	if (block == 0)
		return;
	sb = get_ext2_superblock(dev);
	bp = bread(dev, block);
	for (b = (uint32_t *)bp->b_data; b < (uint32_t *)bp->b_data + (EXT2_BLOCKSIZE(sb) / sizeof(uint32_t)); b++)
		ext2_bfree_dindirect(dev, *b);
	ext2_bfree(dev, block);
	brelse(bp);
}
