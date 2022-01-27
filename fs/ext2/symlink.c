/*
 * ext2/symlink.c
 */
#include <asm/uaccess.h>

#include <drivers/timer.h>
#include <drivers/drive.h>

#include <generic/errno.h>

#include <fs/fs.h>
#include <fs/ext2.h>

#define EXT2_SYMLINK_MAX(sb)	EXT2_BLOCKSIZE(sb)

#define EXT2_FAST_SYMLINK_LEN	60

int
ext2_symlink(const char *path1, const char *path2)
{
	struct ext2_inode_m *ip, *dp;
	const char *name, *p;
	char *s;
	struct ext2_superblock_m *sb;
	struct buffer *bp;
	int err = 0;
	size_t len;

	ip = ext2_namei(path2, &err, &name, &dp, NULL);
	if (ip != NULL) {
		ext2_iput(ip);
		if (ip != dp)
			ext2_iput(dp);
		return -EEXIST;
	}
	if (get_ubyte(name) == '\0') {
		ext2_iput(dp);
		return -ENOTDIR;
	}
	if (err != -ENOENT) {
		ext2_iput(dp);
		return err;
	}
	sb = get_ext2_superblock(dp->i_dev);
	if (sb == NULL) {
		ext2_iput(dp);
		return -EIO;
	}
	for (len = 0, p = path1; get_ubyte(p) != '\0'; p++)
		len++;
	if (len > (size_t)EXT2_SYMLINK_MAX(sb)) {
		ext2_iput(dp);
		return -ENAMETOOLONG;
	}
	ip = ext2_new_file(name, dp, 0777 | EXT2_S_IFLNK, 0, &err);
	if (ip == NULL) {
		ext2_iput(dp);
		return err;
	}
	p = path1;
	if (len < EXT2_FAST_SYMLINK_LEN) {
		s = (char *)&ip->i_ino.i_block[0];
		while (get_ubyte(p) != '\0') {
			*s = (char)get_ubyte(p);
			s++;
			p++;
		}
	} else {
		bp = bread(ip->i_dev, ext2_bmap(ip, 0));
		if (bp == NULL) {
			ext2_unlink(path2);
			ext2_iput(dp);
			return -EIO;
		}
		s = bp->b_data;
		while (get_ubyte(p) != '\0') {
			*s  = (char)get_ubyte(p);
			p++;
			s++;
		}
		ip->i_ino.i_blocks = EXT2_BLOCKSIZE(sb) / SECTOR_SIZE;
		bp->b_flags |= B_DWRITE;
		brelse(bp);
	}
	ip->i_ino.i_mtime = CURRENT_TIME;
	ip->i_flags |= I_MODIFIED;
	ext2_iput(ip);
	return 0;
}
