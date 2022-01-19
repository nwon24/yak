/*
 * ext2/dir.c
 * Routines for adding and removing entries from directories.
 * Just the basic directory structure (linked list) is here.
 */
#include <fs/ext2.h>
#include <fs/fs.h>

#include <drivers/timer.h>

#include <generic/string.h>

#include <kernel/debug.h>

static int dentry_file_type(struct ext2_inode_m *ip);

static int
dentry_file_type(struct ext2_inode_m *ip)
{
	int mode;

	mode = ip->i_ino.i_mode;
	switch (mode & EXT2_S_IFMT) {
	case EXT2_S_IFSOCK:
		return EXT2_FT_SOCK;
	case EXT2_S_IFLNK:
		return EXT2_FT_SYMLINK;
	case EXT2_S_IFREG:
		return EXT2_FT_REG_FILE;
	case EXT2_S_IFDIR:
		return EXT2_FT_DIR;
	case EXT2_S_IFBLK:
		return EXT2_FT_BLKDEV;
	case EXT2_S_IFCHR:
		return EXT2_FT_CHRDEV;
	case EXT2_S_IFIFO:
		return EXT2_FT_FIFO;
	default:
		return EXT2_FT_UNKNOWN;
	}
}

struct ext2_inode_m *
ext2_add_dir_entry(struct ext2_inode_m *dir, struct ext2_inode_m *ip, const char *name, int len)
{
	struct buffer *bp;
	struct ext2_dir_entry *dentry;
	struct ext2_superblock_m *sb;
	uint16_t rec_len;
	ext2_block block, off;

	if (name == NULL || len == 0)
		return NULL;
	/*
	 * Just truncate name if name is too long.
	 * (Who has files with names more than 255 chars anyway?)
	 */
	if (len > EXT2_MAX_NAME_LEN)
		len = EXT2_MAX_NAME_LEN;
	block = ext2_create_block(dir, 0);
	if (block == 0)
		return NULL;
	bp = bread(dir->i_dev, block);
	if (bp == NULL)
		return NULL;
	rec_len = 8 + len;
	if (rec_len % 4 != 0)
		rec_len += 4 - (len % 4);
	sb = get_ext2_superblock(dir->i_dev);
	dentry = (struct ext2_dir_entry *)bp->b_data;
	if (dentry->d_inode == 0) {
		dentry->d_inode = ip->i_num;
		dentry->d_name_len = len;
		dentry->d_rec_len = EXT2_BLOCKSIZE(sb);
		dentry->d_file_type = dentry_file_type(ip);
		brelse(bp);
		return ip;
	}

	off = 0;
	while (1) {
		/*
		 * If we have gone off the end of the block, there is no more space for the
		 * directory entry, so create a new block.
		 */
		if ((char *)dentry >= bp->b_data + EXT2_BLOCKSIZE(sb)) {
			block = ext2_create_block(dir, off);
			brelse(bp);
			bp = bread(dir->i_dev, block);
			if (bp == NULL)
				break;
			dentry = (struct ext2_dir_entry *)bp->b_data;
		}
		/*
		 * An inode value of 0 indicates that the entry is not present.
		 */
		if ((char *)dentry + dentry->d_rec_len >= bp->b_data + EXT2_BLOCKSIZE(sb) && (char *)dentry + rec_len - bp->b_data < EXT2_BLOCKSIZE(sb)) {
			dentry->d_rec_len = dentry->d_name_len + 8;
			if (dentry->d_rec_len % 4 != 0)
				dentry->d_rec_len += 4 - (dentry->d_rec_len % 4);
			dentry = (struct ext2_dir_entry *)((char *)dentry + dentry->d_rec_len);
			dentry->d_inode = ip->i_num;
			dentry->d_file_type = dentry_file_type(ip);
			dentry->d_name_len = len;
			dentry->d_rec_len = EXT2_BLOCKSIZE(sb) - ((char *)dentry - bp->b_data);
			memmove((char *)dentry + 8, name, len);
			bp->b_flags |= B_DWRITE;
			brelse(bp);
			dir->i_ino.i_mtime = CURRENT_TIME;
			ip->i_flags |= I_MODIFIED;
			return ip;
		} else if (dentry->d_inode == 0 || dentry->d_file_type == EXT2_FT_UNKNOWN) {
			block = ext2_create_block(dir, off);
			brelse(bp);
			bp = bread(dir->i_dev, block);
			if (bp == NULL)
				return NULL;
			dentry = (struct ext2_dir_entry *)bp->b_data;
			continue;
		}
		off += dentry->d_rec_len;
		dentry = (struct ext2_dir_entry *)((char *)dentry + dentry->d_rec_len);
	}
	if (bp != NULL)
		brelse(bp);
	return NULL;
}

/*
 * TODO: Implement function to remove entry from directory.
 */
