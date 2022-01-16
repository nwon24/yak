/*
 * ext2/bmap.c
 * Converts logical file offsets into block addresses.
 * Also contains code to allocate a new block to an inode.
 */
#include <fs/ext2.h>
#include <fs/fs.h>

#include <drivers/timer.h>

#include <kernel/debug.h>

static ext2_block bmap_create(struct ext2_inode_m *ip, off_t off, int create);
static ext2_block bmap_create_indirect(struct ext2_inode_m *ip, ext2_block lblk, int create);
static ext2_block bmap_create_dindirect(struct ext2_inode_m *ip, struct ext2_superblock_m *sb, ext2_block lblk, int create);
static ext2_block bmap_create_tindirect(struct ext2_inode_m *ip, struct ext2_superblock_m *sb, ext2_block lblk, int create);

/*
 * This should definitely work.
 * It looks simple enough.
 */
static ext2_block
bmap_create_indirect(struct ext2_inode_m *ip, ext2_block lblk, int create)
{
	struct buffer *bp = NULL;
	ext2_block *b;
	ext2_block ret = 0;

	lblk -= EXT2_DIRECT_BLOCKS + 1;
	b = &ip->i_ino.i_block[EXT2_INDIRECT_BLOCK];
	if (create && *b == 0) {
		*b = ext2_balloc(ip->i_dev, ip->i_num);
		ip->i_flags |= I_MODIFIED;
		ip->i_ino.i_ctime = CURRENT_TIME;
	}
	if (*b == 0)
		goto out;
	bp = bread(ip->i_dev, *b);
	if (bp == NULL)
		goto out;
	if (create && (ret = bp->b_data[lblk]) == 0) {
		bp->b_data[lblk] = ext2_balloc(ip->i_dev, ip->i_num);
		ret = bp->b_data[lblk];
		bp->b_flags |= B_DWRITE;
		ip->i_flags |= I_MODIFIED;
		ip->i_ino.i_ctime = CURRENT_TIME;
	}
out:
	if (bp != NULL)
		brelse(bp);
	return ret;
}

/*
 * I think this works. Hopefully.
 */
static ext2_block
bmap_create_dindirect(struct ext2_inode_m *ip, struct ext2_superblock_m *sb, ext2_block lblk, int create)
{
	struct buffer *bp = NULL;
	ext2_block indir_blocks, *b, tmp;
	ext2_block ret = 0;

	indir_blocks = EXT2_BLOCKSIZE(sb) / sizeof(ext2_block);
	lblk -= EXT2_DIRECT_BLOCKS + indir_blocks+ 1;
	b = &ip->i_ino.i_block[EXT2_DINDIRECT_BLOCK];
	if (create && *b == 0) {
		*b = ext2_balloc(ip->i_dev, ip->i_num);
		ip->i_flags |= I_MODIFIED;
		ip->i_ino.i_ctime = CURRENT_TIME;
	}
	if (*b == 0)
		goto out;
	bp = bread(ip->i_dev, *b);
	if (bp == NULL)
		goto out;
	b = &(((ext2_block *)bp->b_data)[lblk / indir_blocks]);
	if (create && *b == 0) {
		*b = ext2_balloc(ip->i_dev, ip->i_num);
		ip->i_flags |= I_MODIFIED;
		ip->i_ino.i_ctime = CURRENT_TIME;
		bp->b_flags |= B_DWRITE;
	}
	if (*b == 0)
		goto out;
	tmp = *b;
	brelse(bp);
	bp = bread(ip->i_dev, tmp);
	if (bp == NULL)
		goto out;
	b = &(((ext2_block *)bp->b_data)[lblk % indir_blocks]);
	if (create && *b == 0) {
		*b = ext2_balloc(ip->i_dev, ip->i_num);
		ip->i_flags |= I_MODIFIED;
		ip->i_ino.i_ctime = CURRENT_TIME;
		bp->b_flags |= B_DWRITE;
	}
	if (*b == 0)
		goto out;
	ret = *b;
out:
	if (bp != NULL)
		brelse(bp);
	return ret;
}

/*
 * I really have no idea if this works.
 */
static ext2_block
bmap_create_tindirect(struct ext2_inode_m *ip, struct ext2_superblock_m *sb, ext2_block lblk, int create)
{
	struct buffer *bp = NULL;
	ext2_block dindir_blocks, indir_blocks, *b, tmp;
	ext2_block ret = 0;

	indir_blocks = EXT2_BLOCKSIZE(sb) / sizeof(ext2_block);
	dindir_blocks = indir_blocks * indir_blocks;
	lblk -= EXT2_DIRECT_BLOCKS + indir_blocks + dindir_blocks + 1;
	b = &ip->i_ino.i_block[EXT2_TINDIRECT_BLOCK];
	if (create && *b == 0) {
		*b = ext2_balloc(ip->i_dev, ip->i_num);
		ip->i_flags |= I_MODIFIED;
		ip->i_ino.i_ctime = CURRENT_TIME;
	}
	if (*b == 0)
		goto out;
	bp = bread(ip->i_dev, *b);
	if (bp == NULL)
		goto out;
	b = &(((ext2_block *)bp->b_data)[lblk / dindir_blocks]);
	if (create && *b == 0) {
		*b = ext2_balloc(ip->i_dev, ip->i_num);
		ip->i_flags |= I_MODIFIED;
		ip->i_ino.i_ctime = CURRENT_TIME;
		bp->b_flags |= B_DWRITE;
	}
	if (*b == 0)
		goto out;
	tmp = *b;
	brelse(bp);
	bp = bread(ip->i_dev, tmp);
	b = &(((ext2_block *)bp->b_data)[lblk / indir_blocks]);
	if (create && *b == 0) {
		*b = ext2_balloc(ip->i_dev, ip->i_num);
		ip->i_flags |= I_MODIFIED;
		ip->i_ino.i_ctime = CURRENT_TIME;
		bp->b_flags |= B_DWRITE;
	}
	if (*b == 0)
		goto out;
	tmp = *b;
	brelse(bp);
	bp = bread(ip->i_dev, *b);
	if (bp == NULL)
		goto out;
	b = &(((ext2_block *)bp->b_data)[lblk % indir_blocks]);
	if (create && *b == 0) {
		*b = ext2_balloc(ip->i_dev, ip->i_num);
		ip->i_flags |= I_MODIFIED;
		ip->i_ino.i_ctime = CURRENT_TIME;
		bp->b_flags |= B_DWRITE;
	}
	if (*b == 0)
		goto out;
	ret = *b;
out:
	if (bp != NULL)
		brelse(bp);
	return ret;
}

/*
 * Looks simple enough, but once you get into doubly indirect blocks
 * and then triply indirect blocks things become a mess quickly.
 */
static ext2_block
bmap_create(struct ext2_inode_m *ip, off_t off, int create)
{
	/* Logical block offset */
	ext2_block lblk, *b;
	ext2_block indir_blocks, dindir_blocks;
	struct ext2_superblock_m *sb;

	sb = get_ext2_superblock(ip->i_dev);
	/*
	 * First figure out the logical block offset in the file from the byte offset.
	 * Block offsets start at 0.
	 * i.e, if 'off' were 0, then 'lblk' would be block 0 (or the first block in the file).
	 */
	lblk = off / EXT2_BLOCKSIZE(sb);
	if (lblk <= EXT2_DIRECT_BLOCKS) {
		b = &ip->i_ino.i_block[lblk];
		if (*b == 0 && create) {
			*b = ext2_balloc(ip->i_dev, ip->i_num);
			ip->i_flags |= I_MODIFIED;
			ip->i_ino.i_ctime = CURRENT_TIME;
		}
		return *b;
	}
	/*
	 * Figure out the level of indirection needed.
	 * '<' is needed because remember than 'lblk' starts off at '0',
	 * so 'lblk' == 268 is actually the 269th block of the file, and thus
	 * needs double indirection.
	 */
	indir_blocks = EXT2_BLOCKSIZE(sb) / sizeof(ext2_block);
	if (lblk < indir_blocks + EXT2_DIRECT_BLOCKS + 1)
		return bmap_create_indirect(ip, lblk, create);
	dindir_blocks = indir_blocks * indir_blocks;
	if (lblk < dindir_blocks + indir_blocks + EXT2_DIRECT_BLOCKS + 1)
		return bmap_create_dindirect(ip, sb, lblk, create);
	return bmap_create_tindirect(ip, sb, lblk, create);
}

/*
 * Used in read routines because if the block is not there, we want to return
 * an error rather than creating it automatically.
 */
ext2_block
ext2_bmap(struct ext2_inode_m *ip, off_t off)
{
	return bmap_create(ip, off, 0);
}

/*
 * Used in write routines because the file should be naturally expanded as
 * more data is written to it.
 */
ext2_block
ext2_create_block(struct ext2_inode_m *ip, off_t off)
{
	return bmap_create(ip, off, 1);
}
