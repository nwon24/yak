/*
 * ext2/namei.c
 * Converts a path name into an inode.
 * This is very badly written, as this was
 * written before 'ext2_bmap' was written.
 * Hopefully it works.
 */
#include <asm/types.h>
#include <asm/uaccess.h>

#include <fs/buffer.h>
#include <fs/ext2.h>
#include <fs/fs.h>

#include <generic/errno.h>
#include <generic/string.h>

#include <kernel/proc.h>
#include <kernel/debug.h>

#define EXT2_MAX_SYMLINKS	5

static int symlinks = 0;

/*
 * 'FIND_NEXT_BLOCK' means it couldn't find the directory entry.
 * 'FIND_NONE' means it hit the end of the directory linked list.
 */
enum {
	FIND_NEXT_BLOCK,
	FIND_NONE
};

static struct ext2_inode_m *find_entry(struct ext2_inode_m *dir, const char *entry, size_t len, struct buffer **last_dir_bp);
static struct ext2_inode_m *find_entry_in_block(struct ext2_inode_m *dir, ssize_t block, const char *entry, size_t len, int *res, struct buffer **last_dir_bp);
static struct ext2_inode_m *find_entry_direct(struct ext2_inode_m *dir, const char *entry, size_t len, int *res, struct buffer **last_dir_bp);
static struct ext2_inode_m *find_entry_indirect(struct ext2_inode_m *dir, ssize_t block, const char *entry, size_t len, int *res, struct buffer **last_dir_bp);
static struct ext2_inode_m *find_entry_dindirect(struct ext2_inode_m *dir, ssize_t block, const char *entry, size_t len, int *res, struct buffer **last_dir_bp);
static struct ext2_inode_m *find_entry_tindirect(struct ext2_inode_m *dir, ssize_t block, const char *entry, size_t len, int *res, struct buffer **last_dir_bp);

/*
 * Can only use real UID and real GID here.
 */
int
ext2_access(const char *path, int amode)
{
	struct ext2_inode_m *ip;
	mode_t mode;
	int err, ret;

	if (current_process->uid == 0)
		return 0;
	ip = ext2_namei(path, &err, NULL, NULL, NULL, NULL, NULL);
	if (ip == NULL)
		return err;
	mode = ip->i_ino.i_mode;
	if (current_process->uid == ip->i_ino.i_uid)
		mode >>= 6;
	else if (current_process->gid == ip->i_ino.i_gid)
		mode >>= 3;
	if ((mode & amode) != 0)
		ret = 0;
	else
		ret = -EACCES;
	ext2_iput(ip);
	return ret;
}

int
ext2_permission(struct ext2_inode_m *ip, int mask)
{
	int mode = ip->i_ino.i_mode;

	if (!(current_process->uid && current_process->euid))
		/* Super user, nothing matters. */
		return 0;
	if (current_process->uid == ip->i_ino.i_uid || current_process->euid == ip->i_ino.i_uid)
		mode >>= 6;
	else if (current_process->gid == ip->i_ino.i_gid || current_process->egid == ip->i_ino.i_gid)
		mode >>= 3;
	mode &= 7;
	if ((mode & mask) != 0)
		return 0;
	return -1;
}

int
ext2_public_permission(void *inode, int mask)
{
	return ext2_permission(inode, mask);
}

void *
ext2_public_namei(const char *path, int *err)
{
	*err = 0;
	return ext2_namei(path, err, NULL, NULL, NULL, NULL, NULL);
}

struct ext2_inode_m *
ext2_namei(const char *path,
	   int *error,
	   const char **base,
	   struct ext2_inode_m **last_dir,
	   struct buffer **last_dir_bp,
	   struct ext2_inode_m *root,
	   struct ext2_inode_m *cwd)
{
	struct ext2_inode_m *ip, *dp, *start, *prev_dir = NULL;
	const char *p1, *p2;

	if (current_process->root_inode.inode == NULL || current_process->cwd_inode.inode == NULL)
		panic("Current process has no root directory or working directory");
	if (current_process->root_fs->f_fs != EXT2 && current_process->root_fs->f_fs != EXT2) {
		/*
		 * Should probably panic here.
		 * Probably not the best erro code.
		 */
		*error = -EINVAL;
		return NULL;
	}
	if (root == NULL)
		root = current_process->root_inode.inode;
	if (cwd == NULL)
		cwd = current_process->cwd_inode.inode;
	p1 = path;
	ip = (get_ubyte(p1) == '/') ? root : cwd;
	if (ip == NULL)
		panic("ext2_namei: current_process has no root inode or working directory inode");
	if (last_dir != NULL)
		*last_dir = ip;
	else
		prev_dir = ip;
	ip->i_count++;
	start = ip;
	symlinks = 0;
loop:
	if (get_ubyte(p1) != '\0' && !EXT2_S_ISDIR(ip->i_ino.i_mode) && !EXT2_S_ISLNK(ip->i_ino.i_mode)) {
		if (last_dir != NULL && *last_dir != ip)
			ext2_iput(ip);
		*error = -ENOTDIR;
		return NULL;
	}
	if (get_ubyte(p1) == '\0'
	    || (get_ubyte(p1) == '/' && get_ubyte(p1 + 1) == '\0')) {
		if (prev_dir != NULL && prev_dir != ip)
			ext2_iput(prev_dir);
		symlinks = 0;
		return ip;
	}
	if (EXT2_S_ISLNK(ip->i_ino.i_mode)) {
		/*
		 * Just assume that if we are doing more than 5 symlinks we are in a loop.
		 * Who uses paths that have 5 symlinks in them?
		 * Who uses symlinks anyway?
		 * See harmful.cat-v.org/software/symlinks.
		 */
		if (symlinks >= EXT2_MAX_SYMLINKS) {
			ext2_iput(ip);
			if (prev_dir != NULL && prev_dir != ip)
				ext2_iput(prev_dir);
			*error = -ELOOP;
			symlinks = 0;
			return NULL;
		}
		/*
		 * WARNING: 'ext2_follow_symlink' calls 'ext2_namei'.
		 * It is in a separate function because it also has to figure out if it is a 'fast'
		 * symlink or the symlink is stored in a block.
		 */
		dp = ext2_follow_symlink(ip, root, (last_dir == NULL) ? prev_dir : *last_dir, error);
		if (dp == NULL) {
			ext2_iput(ip);
			if (prev_dir != ip)
				ext2_iput(prev_dir);
			symlinks = 0;
			return NULL;
		}
		symlinks++;
		ip = dp;
	}
	if (ext2_permission(ip, PERM_SRCH) < 0) {
		if (last_dir != NULL && *last_dir != ip)
			ext2_iput(ip);
		if (prev_dir != NULL && prev_dir != ip)
			ext2_iput(prev_dir);
		*error = -EACCES;
		symlinks = 0;
		return NULL;
	}
	while (get_ubyte(p1) == '/')
		p1++;
	p2 = p1;
	if (base != NULL)
		*base = p2;
	while (get_ubyte(p1) != '/' && get_ubyte(p1) != '\0')
		p1++;
	if (ip != start) {
		if (last_dir != NULL && EXT2_S_ISDIR(ip->i_ino.i_mode)) {
			if (*last_dir != NULL)
				ext2_iput(*last_dir);
			*last_dir = ip;
		} else {
			ext2_iput(prev_dir);
			prev_dir = ip;
		}
	}
	if ((dp = find_entry(ip, p2, p1 - p2, last_dir_bp)) == NULL) {
		*error = -ENOENT;
		if (prev_dir != NULL)
			ext2_iput(prev_dir);
		symlinks = 0;
		return NULL;
	}
	ip = dp;
	goto loop;
}

static struct ext2_inode_m *
find_entry(struct ext2_inode_m *dir, const char *entry, size_t len, struct buffer **last_dir_bp)
{
	struct ext2_inode_m *ip;
	int res;

	if ((ip = find_entry_direct(dir, entry, len, &res, last_dir_bp)) != NULL)
		return ip;
	if (res == FIND_NONE)
		return NULL;
	if ((ip = find_entry_indirect(dir, dir->i_ino.i_block[EXT2_INDIRECT_BLOCK], entry, len, &res, last_dir_bp)) != NULL)
		return ip;
	if (res == FIND_NONE)
		return NULL;
	if ((ip = find_entry_dindirect(dir, dir->i_ino.i_block[EXT2_DINDIRECT_BLOCK], entry, len, &res, last_dir_bp)) != NULL)
		return ip;
	if (res == FIND_NONE)
		return NULL;
	if ((ip = find_entry_tindirect(dir, dir->i_ino.i_block[EXT2_TINDIRECT_BLOCK], entry, len, &res, last_dir_bp)) != NULL)
		return ip;
	return NULL;
}

static struct ext2_inode_m *
find_entry_direct(struct ext2_inode_m *dir, const char *entry, size_t len, int *res, struct buffer **last_dir_bp)
{
	int i;
	struct ext2_inode_m *ip;

	for (i = 0; i <= EXT2_DIRECT_BLOCKS; i++) {
		if (dir->i_ino.i_block[i] == 0) {
			*res = FIND_NONE;
			return NULL;
		}
		if ((ip = find_entry_in_block(dir, dir->i_ino.i_block[i], entry, len, res, last_dir_bp)) != NULL)
			return ip;
	}
	*res = FIND_NEXT_BLOCK;
	return NULL;
}

static struct ext2_inode_m *
find_entry_indirect(struct ext2_inode_m *dir, ssize_t block, const char *entry, size_t len, int *res, struct buffer **last_dir_bp)
{
	struct buffer *bp;
	struct ext2_inode_m *ip = NULL;
	struct ext2_superblock_m *sb;
	uint32_t *p;

	if (block == 0)
		return NULL;
	sb = get_ext2_superblock(dir->i_dev);
	bp = bread(dir->i_dev, block);
	for (p = (uint32_t *)bp->b_data; (char *)p < bp->b_data + EXT2_BLOCKSIZE(sb); p++) {
		if (*p == 0)
			goto out;
		if ((ip = find_entry_in_block(dir, *p, entry, len, res, last_dir_bp)) != NULL)
			goto out;
	}
out:
	brelse(bp);
	return ip;
}

static struct ext2_inode_m *
find_entry_dindirect(struct ext2_inode_m *dir, ssize_t block, const char *entry, size_t len, int *res, struct buffer **last_dir_bp)
{
	struct buffer *bp;
	struct ext2_inode_m *ip = NULL;
	struct ext2_superblock_m *sb;
	uint32_t *p;

	if (block == 0)
		return NULL;
	sb = get_ext2_superblock(dir->i_dev);
	bp = bread(dir->i_dev, block);
	for (p = (uint32_t *)bp->b_data; (char *)p < bp->b_data + EXT2_BLOCKSIZE(sb); p++) {
		if (*p == 0)
			goto out;
		if ((ip = find_entry_indirect(dir, *p, entry, len, res, last_dir_bp)) != NULL)
			goto out;
	}
out:
	brelse(bp);
	return ip;

}

static struct ext2_inode_m *
find_entry_tindirect(struct ext2_inode_m *dir, ssize_t block, const char *entry, size_t len, int *res, struct buffer **last_dir_bp)
{
	struct buffer *bp;
	struct ext2_inode_m *ip = NULL;
	struct ext2_superblock_m *sb;
	uint32_t *p;

	if (block == 0)
		return NULL;
	sb = get_ext2_superblock(dir->i_dev);
	bp = bread(dir->i_dev, block);
	for (p = (uint32_t *)bp->b_data; (char *)p < bp->b_data + EXT2_BLOCKSIZE(sb); p++) {
		if (*p == 0)
			goto out;
		if ((ip = find_entry_dindirect(dir, *p, entry, len, res, last_dir_bp)) != NULL)
			goto out;
	}
out:
	brelse(bp);
	return ip;

}

static struct ext2_inode_m *
find_entry_in_block(struct ext2_inode_m *dir, ssize_t block, const char *entry, size_t len, int *res, struct buffer **last_dir_bp)
{
	struct buffer *bp;
	struct ext2_superblock_m *sb;
	struct ext2_inode_m *ret = NULL;
	struct ext2_dir_entry *d;
	int flag;

	if (block == 0)
		return NULL;
	sb = get_ext2_superblock(dir->i_dev);
	bp = bread(dir->i_dev, block);
	d = (struct ext2_dir_entry *)bp->b_data;

	while ((flag = ext2_match(d, entry, len)) != 0) {
		if ((char *)d - bp->b_data >= EXT2_BLOCKSIZE(sb)) {
			*res = FIND_NEXT_BLOCK;
			d = NULL;
			break;
		}
		d = (struct ext2_dir_entry *)((char *)d + d->d_rec_len);
		if (d->d_inode == 0 || d->d_file_type == EXT2_FT_UNKNOWN) {
			*res = FIND_NONE;
			break;
		}
	}
	if (flag == 0)
		ret = ext2_iget(dir->i_dev, d->d_inode);
	if (last_dir_bp != NULL && ret != NULL) {
		*last_dir_bp = bp;
	} else if (last_dir_bp != NULL && ret == NULL) {
		*last_dir_bp = NULL;
		brelse(bp);
	} else {
		brelse(bp);
	}
	return ret;
}

int
ext2_match(struct ext2_dir_entry *dentry, const void *uptr, size_t n)
{
	const unsigned char *a, *b;

	if (n != dentry->d_name_len)
		return -1;
	a = (const unsigned char *)dentry + 8;
	b = (const unsigned char *)uptr;
	while (n--) {
		if (*a < get_ubyte(b))
			return -1;
		else if (get_ubyte(b) < *a)
			return 1;
		a++;
		b++;
	}
	return 0;
}

/*
 * This is here because it seems to belong nowhere else.
 * Won't be fun debugging this one.
 * TODO: Check for 'old' being an ancestor of 'new'.
 */
int
ext2_rename(const char *old, const char *new)
{
	struct ext2_inode_m *ipold, *ipnew, *dpold, *dpnew;
	const char *pold, *pnew, *tmp;
	struct buffer *bp;
	int err;

	bp = NULL;
	ipold = ext2_namei(old, &err, &pold, &dpold, &bp, NULL, NULL);
	if ((get_ubyte(pold) == '.' && get_ubyte(pold + 1) == '.' && get_ubyte(pold + 2) == '\0')
	    || (get_ubyte(pold) == '.' && get_ubyte(pold + 1) == '\0')) {
		if (ipold != NULL)
			ext2_iput(ipold);
		if (dpold != ipold)
			ext2_iput(dpold);
		if (bp != NULL)
			brelse(bp);
		return -EINVAL;
	}
	if (ipold == NULL) {
		if (err != -ENOENT) {
			if (bp != NULL)
				brelse(bp);
			ext2_iput(dpold);
			return err;
		}
		for (tmp = pold; get_ubyte(tmp) != '\0' && get_ubyte(tmp) != '/'; tmp++);
		if (get_ubyte(tmp) == '\0') {
			ext2_iput(dpold);
			if (bp != NULL)
				brelse(bp);
			return -ENOENT;
		} else {
			ipold = dpold;
		}
	}
	ipnew = ext2_namei(new, &err, &pnew, &dpnew, NULL, NULL, NULL);
	if (ipnew != NULL) {
		if (!EXT2_S_ISDIR(ipnew->i_ino.i_mode)) {
			err = -EEXIST;
			goto end;
		}
		if (ext2_empty_dir(ipnew, &err) != 0) {
			if (err != -EIO)
				err = -ENOTEMPTY;
			goto end;
		}
	}
	ext2_remove_dir_entry(pold, ipold, bp);
	ext2_add_dir_entry(dpnew, ipold, pnew, strlen(pnew));
	err = 0;
end:
	if (bp != NULL)
		brelse(bp);
	ext2_iput(ipold);
	if (ipold != dpold)
		ext2_iput(dpold);
	ext2_iput(dpnew);
	return err;
}
