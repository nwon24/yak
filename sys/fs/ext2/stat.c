/*
 * ext2/stat.c
 * Contains the 'stat' and 'fstat' system call for ext2.
 */
#include <asm/uaccess.h>

#include <fs/ext2.h>

#include <generic/stat.h>
#include <generic/errno.h>

#include <kernel/proc.h>

static int _ext2_stat(struct ext2_inode_m *ip, struct stat *statp);

static int
_ext2_stat(struct ext2_inode_m *ip, struct stat *statp)
{
	struct stat tmp;
	struct ext2_superblock_m *sb;

	sb = get_ext2_superblock(ip->i_dev);
	if (sb == NULL)
		return -EINVAL;
	tmp.st_dev = ip->i_dev;
	tmp.st_ino = ip->i_num;
	tmp.st_mode = ip->i_ino.i_mode;
	tmp.st_nlink = ip->i_ino.i_links_count;
	tmp.st_uid = ip->i_ino.i_uid;
	tmp.st_gid = ip->i_ino.i_gid;
	tmp.st_rdev = ip->i_ino.i_block[0];
	tmp.st_size = ip->i_ino.i_size;
	tmp.st_atime = ip->i_ino.i_atime;
	tmp.st_ctime = ip->i_ino.i_ctime;
	tmp.st_mtime = ip->i_ino.i_mtime;
	tmp.st_blksize = EXT2_BLOCKSIZE(sb);
	tmp.st_blocks = ip->i_ino.i_blocks;
	cp_to_user(statp, &tmp, sizeof(tmp));
	return 0;
}

int
ext2_stat(const char *path, struct stat *statp)
{
	struct ext2_inode_m *ip, *dp, *tmp;
	int err = 0;

	ip = ext2_namei(path, &err, NULL, &dp, NULL, NULL, NULL);
	if (ip == NULL) {
		ext2_iput(dp);
		return err;
	}
	if (EXT2_S_ISLNK(ip->i_ino.i_mode)) {
		tmp = ext2_follow_symlink(ip, current_process->root_inode.inode, dp, &err);
		if (tmp == NULL && err == -EIO)
			goto out;
		ext2_iput(ip);
		ip = tmp;
	}
	err = _ext2_stat(ip, statp);
out:
	ext2_iput(ip);
	if (ip != dp)
		ext2_iput(dp);
	return err;
}

int
ext2_lstat(const char *path, struct stat *statp)
{
	struct ext2_inode_m *ip;
	int err;

	ip = ext2_namei(path, &err, NULL, NULL, NULL, NULL, NULL);
	if (ip == NULL)
		return err;
	return _ext2_stat(ip, statp);
}

int
ext2_fstat(struct file *file, struct stat *statp)
{
	return _ext2_stat(file->f_inode, statp);
}
