/*
 * ext2/dir.c
 * Routines for adding and removing entries from directories.
 * Just the basic directory structure (linked list) is here.
 * Contains system calls 'link' and 'unlink' for ext2.
 */
#include <asm/uaccess.h>

#include <fs/ext2.h>
#include <fs/fs.h>

#include <drivers/timer.h>

#include <generic/string.h>
#include <generic/errno.h>

#include <kernel/debug.h>
#include <kernel/proc.h>

static int dentry_file_type(struct ext2_inode_m *ip);
static int empty_dir(struct ext2_inode_m *dp, int *err);

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
ext2_empty_dir(struct ext2_inode_m *dp, int *err)
{
	return empty_dir(dp, err);
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
	if (dentry->d_inode == 0 || dir->i_ino.i_size == 0) {
		dentry->d_inode = ip->i_num;
		dentry->d_name_len = len;
		dentry->d_rec_len = EXT2_BLOCKSIZE(sb);
		dentry->d_file_type = dentry_file_type(ip);
		memmove((char *)dentry + 8, name, len);
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
			dir->i_ino.i_size += EXT2_BLOCKSIZE(sb);
			dir->i_flags |= I_MODIFIED;
			dir->i_ino.i_mtime = CURRENT_TIME;
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
int
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
		bp->b_flags |= B_DWRITE;
		return 0;
	}
	dentry = (struct ext2_dir_entry *)((char *)dentry + dentry->d_rec_len);
	while (dentry < (struct ext2_dir_entry *)(bp->b_data + EXT2_BLOCKSIZE(sb))) {
		if (dentry->d_inode == ip->i_num && ext2_match(dentry, name, strlen(name)) == 0) {
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

/*
 * Checks to see if the specified directory is empty.
 * By 'empty', meaning it only consists of '.' and '..'
 * 'err' is for an error from errno.h.
 */
static int
empty_dir(struct ext2_inode_m *dp, int *err)
{
	struct buffer *bp;
	struct ext2_dir_entry *dentry;
	struct ext2_superblock_m *sb;
	int not_empty = 0;

	bp = bread(dp->i_dev, dp->i_ino.i_block[0]);
	if (bp == NULL) {
		*err = -EIO;
		return 1;
	}
	sb = get_ext2_superblock(dp->i_dev);
	dentry = (struct ext2_dir_entry *)bp->b_data;
	if (ext2_match(dentry, ".", 1) != 0) {
		printk("WARNING: ext2 directory with inode number %d does not have the first entry as '.'. Filesystem corrupt.\r\n", dp->i_num);
		not_empty = 1;
		goto out;
	}
	dentry = (struct ext2_dir_entry *)((char *)dentry + dentry->d_rec_len);
	if (dentry >= (struct ext2_dir_entry *)(bp->b_data + EXT2_BLOCKSIZE(sb))) {
		printk("WARNING: ext2 directory with inode number %d is missing '..'. Filesystem corrupt.\r\n", dp->i_num);
		not_empty = 1;
		goto out;
	}
	if (ext2_match(dentry, "..", 2) != 0) {
		printk("WARNING: ext2 directory with inode number %d is missing '..'. Filesystem corrupt.\r\n", dp->i_num);
		not_empty = 1;
		goto out;
	}
	dentry = (struct ext2_dir_entry *)((char *)dentry + dentry->d_rec_len);
	if (dentry < (struct ext2_dir_entry *)(bp->b_data + EXT2_BLOCKSIZE(sb))) {
		/* There are other entries. */
		not_empty = 1;
		goto out;
	}
	not_empty = 0;
 out:
	brelse(bp);
	return not_empty;
}

int
ext2_unlink(const char *pathname)
{
	struct ext2_inode_m *ip, *dp;
	struct buffer *bp;
	const char *p;
	int err;

	ip = ext2_namei(pathname, &err, &p, &dp, &bp, NULL, NULL);
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
	if (ip == current_process->root_inode.inode) {
		ext2_iput(ip);
		if (ip != dp)
			ext2_iput(dp);
		return -EBUSY;
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

	ip1 = ext2_namei(path1, &err, NULL, NULL, NULL, NULL, NULL);
	if (ip1 == NULL)
		return err;
	if (EXT2_S_ISDIR(ip1->i_ino.i_mode) || EXT2_S_ISLNK(ip1->i_ino.i_mode)) {
		/*
		 * Do not support linking of directories or symbolic links.
		 */
		ext2_iput(ip1);
		return -EEXIST;
	}
	ip2 = ext2_namei(path2, &err, &p, &dp, NULL, NULL, NULL);
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

	ip = ext2_namei(path, &err, NULL, &dp, NULL, NULL, NULL);
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
	ext2_iput(current_process->cwd_inode.inode);
	current_process->cwd_inode.inode = ip;
	return 0;
}

int
ext2_mkdir(const char *path, mode_t mode)
{
	mode_t m;
	struct ext2_inode_m *ip, *dp;
	const char *p, *tmp;
	struct ext2_superblock_m *sb;
	struct ext2_blk_group_desc *bgd;
	int err;

	m = (mode & ~current_process->umask & 0777) | EXT2_S_IFDIR;
	ip = ext2_namei(path, &err, &p, &dp, NULL, NULL, NULL);
	if (ip == NULL) {
		for (tmp = p ; get_ubyte(tmp) != '\0' && get_ubyte(tmp) != '/'; tmp++);
		if (get_ubyte(tmp) == '/' && get_ubyte(tmp + 1) != '\0') {
			ext2_iput(dp);
			return -ENOENT;
		} else if (get_ubyte(tmp) == '/' && get_ubyte(tmp + 1) == '\0') {
			put_ubyte(tmp, '\0');
		}
	} else {
		ext2_iput(ip);
		return -EEXIST;
	}
	if (ext2_permission(dp, PERM_WRITE) < 0) {
		ext2_iput(dp);
		return -EACCES;
	}
	ip = ext2_new_file(p, dp, m, 0, &err);
	if (ip == NULL) {
		ext2_iput(dp);
		return err;
	}
	sb = get_ext2_superblock(ip->i_dev);
	ext2_add_dir_entry(ip, ip, ".", 1);
	ip->i_ino.i_size = EXT2_BLOCKSIZE(sb);
	ip->i_ino.i_links_count++;
	ext2_add_dir_entry(ip, dp, "..", 2);
	dp->i_ino.i_links_count++;
	dp->i_flags |= I_MODIFIED;
	dp->i_ino.i_mtime = CURRENT_TIME;
	ip->i_flags |= I_MODIFIED;
	ip->i_ino.i_mtime = CURRENT_TIME;
	bgd = sb->bgd_table + EXT2_BLOCK_GROUP(ip, sb);
	mutex_lock(&sb->mutex);
	bgd->bg_used_dirs_count++;
	sb->modified = 1;
	mutex_unlock(&sb->mutex);
	ext2_iput(ip);
	if (ip != dp)
		ext2_iput(dp);
	return 0;
}

int
ext2_rmdir(const char *path)
{
	struct ext2_inode_m *ip, *dp;
	struct ext2_superblock_m *sb;
	struct buffer *bp = NULL;
	const char *p, *tmp;
	int err = 0, ret;

	ret = 0;
	ip = ext2_namei(path, &err, &p, &dp, &bp, NULL, NULL);
	if (ip == NULL) {
		ext2_iput(dp);
		return err;
	}
	tmp = p;
	if (get_ubyte(tmp) == '.' && get_ubyte(tmp + 1) == '\0') {
		ret = -EINVAL;
		goto out;
	}
	if (get_ubyte(tmp) == '.' && get_ubyte(tmp + 1) == '.') {
		ret = -ENOTEMPTY;
		goto out;
	}
	if (empty_dir(ip, &err) != 0) {
		ret = (err != 0) ? err : -ENOTEMPTY;
		goto out;
	}
	if (ip == current_process->root_inode.inode) {
		ret = -EBUSY;
		goto out;
	}
	if (ext2_permission(dp, PERM_WRITE) < 0) {
		ret = -EACCES;
		goto out;
	}
	if (!EXT2_S_ISDIR(ip->i_ino.i_mode)) {
		ret = -ENOTDIR;
		goto out;
	}
	if ((ip->i_ino.i_mode & EXT2_S_ISVTX)
	    && (current_process->euid != ip->i_ino.i_uid && current_process->euid != dp->i_ino.i_uid)) {
		ret = -EPERM;
		goto out;
	}
	sb = get_ext2_superblock(ip->i_dev);
	if (ext2_remove_dir_entry(p, ip, bp) < 0) {
		ret = -ENOENT;
		goto out;
	}
	ip->i_ino.i_links_count -= 2;
	ip->i_flags |= I_MODIFIED;
	ip->i_ino.i_mtime = CURRENT_TIME;
	if (ip != dp) {
		dp->i_ino.i_links_count--;
		dp->i_flags |= I_MODIFIED;
		dp->i_ino.i_mtime = CURRENT_TIME;
	}
	mutex_lock(&sb->mutex);
	sb->bgd_table[EXT2_BLOCK_GROUP(ip, sb)].bg_used_dirs_count--;
	sb->modified = 1;
	mutex_unlock(&sb->mutex);
out:
	if (bp != NULL)
		brelse(bp);
	ext2_iput(ip);
	if (dp != ip)
		ext2_iput(dp);
	return ret;
}
