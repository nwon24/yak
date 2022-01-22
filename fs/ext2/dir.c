/*
 * ext2/dir.c
 * Routines for adding and removing entries from directories.
 * Just the basic directory structure (linked list) is here.
 * Contains system calls 'link' and 'unlink' for ext2.
 */
#include <fs/ext2.h>
#include <fs/fs.h>

#include <drivers/timer.h>

#include <generic/string.h>
#include <generic/errno.h>

#include <kernel/debug.h>
#include <kernel/proc.h>

static int dentry_file_type(struct ext2_inode_m *ip);
static int ext2_remove_dir_entry(const char *name, struct ext2_inode_m *ip, struct buffer *bp);

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

int
ext2_add_dir_entry(struct ext2_inode_m *dir, struct ext2_inode_m *ip, const char *name, int len)
{
	struct buffer *bp = NULL;
	struct ext2_dir_entry *dentry;
	struct ext2_superblock_m *sb;
	uint16_t rec_len;
	ext2_block block, off;
	int ret = 0;

	if (name == NULL || len == 0) {
		ret = -EINVAL;
		goto error;
	}
	/*
	 * Just truncate name if name is too long.
	 * (Who has files with names more than 255 chars anyway?)
	 */
	if (len > EXT2_MAX_NAME_LEN)
		len = EXT2_MAX_NAME_LEN;
	block = ext2_create_block(dir, 0);
	if (block == 0) {
		ret = -ENOSPC;
		goto error;
	}
	bp = bread(dir->i_dev, block);
	if (bp == NULL) {
		ret = -ENOSPC;
		goto error;
	}
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
		ret = 0;
		goto error;
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
			if (bp == NULL) {
				ret = -EIO;
				break;
			}
			dentry = (struct ext2_dir_entry *)bp->b_data;
		}
		/*
		 * An inode value of 0 indicates that the entry is not present.
		 */
		if ((char *)dentry + dentry->d_rec_len >= bp->b_data + EXT2_BLOCKSIZE(sb)
		    && (dentry->d_rec_len >= rec_len)) {
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
			ret = 0;
			goto error;
		} else if ((char *)dentry + dentry->d_rec_len >= bp->b_data + EXT2_BLOCKSIZE(sb)) {
			block = ext2_create_block(dir, off);
			brelse(bp);
			bp = bread(dir->i_dev, block);
			if (bp == NULL) {
				ret = -EIO;
				goto error;
			}
			dentry = (struct ext2_dir_entry *)bp->b_data;
			continue;
		}
		off += dentry->d_rec_len;
		dentry = (struct ext2_dir_entry *)((char *)dentry + dentry->d_rec_len);
	}
error:
	if (bp != NULL)
		brelse(bp);
	return ret;
}

/*
 * Removes the directory entry from the block pointed to by 'bp'.
 * Need to check the entry name as well as a directory could have
 * two links to the same inode under different names.
 */
static int
ext2_remove_dir_entry(const char *name, struct ext2_inode_m *ip, struct buffer *bp)
{
	struct ext2_dir_entry *dentry, *prev;
	struct ext2_superblock_m *sb;

	sb = get_ext2_superblock(ip->i_dev);
	dentry = prev = (struct ext2_dir_entry *)bp->b_data;
	if (dentry->d_inode == ip->i_num) {
		/*
		 * This means it is at the start of the block.
		 * According nongnu.org/ext2-doc/ext2.html, if the entry at the start of the
		 * block is removed, it is simply blanked and the record length set to the end of
		 * the block. The block is not deallocated.
		 */
		memset(dentry, 0, sizeof(*dentry));
		dentry->d_rec_len = EXT2_BLOCKSIZE(sb);
		return 0;
	}
	dentry = (struct ext2_dir_entry *)((char *)dentry + dentry->d_rec_len);
	while (dentry < (struct ext2_dir_entry *)(bp->b_data + EXT2_BLOCKSIZE(sb))) {
		if (dentry->d_inode == ip->i_num && ext2_match((char *)dentry + 8, name, strlen(name)) == 0) {
			/* Bingo. */
			prev->d_rec_len += dentry->d_rec_len;
			bp->b_flags |= B_DWRITE;
			return 0;
		}
		prev = dentry;
		dentry = (struct ext2_dir_entry *)((char *)dentry + dentry->d_rec_len);
	}
	return -ENOENT;
}

int
ext2_unlink(const char *pathname)
{
	struct ext2_inode_m *ip, *dp;
	struct buffer *bp;
	const char *p;
	int err;

	ip = ext2_namei(pathname, &err, &p, &dp, &bp);
	if (ip == NULL) {
		if (bp != NULL)
			brelse(bp);
		return err;
	}
	if (ext2_permission(dp, PERM_WRITE) < 0) {
		ext2_iput(ip);
		if (ip != dp)
			ext2_iput(dp);
		return -EACCES;
	}
	err = ext2_remove_dir_entry(p, ip, bp);
	brelse(bp);
	ip->i_ino.i_links_count--;
	ip->i_ino.i_mtime = CURRENT_TIME;
	ip->i_flags |= I_MODIFIED;
	ext2_iput(ip);
	if (ip != dp)
		ext2_iput(dp);
	return err;
}

int
ext2_link(const char *path1, const char *path2)
{
	struct ext2_inode_m *ip1, *ip2;
	struct ext2_inode_m *dp;
	const char *p;
	int err;

	ip1 = ext2_namei(path1, &err, NULL, NULL, NULL);
	if (ip1 == NULL)
		return err;
	if (EXT2_S_ISDIR(ip1->i_ino.i_mode) || EXT2_S_ISLNK(ip1->i_ino.i_mode)) {
		/*
		 * Do not support linking of directories of symbolic links.
		 */
		ext2_iput(ip1);
		return -EEXIST;
	}

	ip2 = ext2_namei(path2, &err, &p, &dp, NULL);
	if (ip2 != NULL) {
		if (ip1 != dp)
			ext2_iput(dp);
		ext2_iput(ip1);
		return -EEXIST;
	}
	err = ext2_add_dir_entry(dp, ip1, p, strlen(p));
	ip1->i_ino.i_links_count++;
	ip1->i_ino.i_mtime = CURRENT_TIME;
	ip1->i_flags |= I_MODIFIED;
	ext2_iput(ip1);
	if (ip1 != dp)
		ext2_iput(dp);
	return err;
}

int
ext2_chdir(const char *path)
{
	struct ext2_inode_m *ip, *dp;
	int err;

	ip = ext2_namei(path, &err, NULL, &dp, NULL);
	if (path[strlen(path)] == '/') {
		if (!EXT2_S_ISDIR(dp->i_ino.i_mode)) {
			ext2_iput(dp);
			return -ENOTDIR;
		}
	}
	if (ip == NULL) {
		ext2_iput(dp);
		return err;
	}
	ext2_iput(current_process->cwd_inode);
	current_process->cwd_inode = ip;
	return 0;
}
