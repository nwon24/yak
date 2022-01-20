/*
 * ext2/read_write.c
 * Implements the 'read' and 'write' system calls for ext2.
 * Also has 'lseek', as that is somewhat related.
 */
#include <asm/types.h>
#include <asm/uaccess.h>

#include <drivers/timer.h>

#include <fs/dev.h>
#include <fs/fs.h>
#include <fs/ext2.h>

#include <generic/string.h>
#include <generic/errno.h>
#include <generic/fcntl.h>

#include <kernel/debug.h>
#include <kernel/mutex.h>

static ssize_t file_read(struct file *file, struct ext2_inode_m *ip, void *buf, size_t count);
static ssize_t file_write(struct file *file, struct ext2_inode_m *ip, void *buf, size_t count);

int
ext2_read(struct file *file, void *buf, size_t count)
{
	struct ext2_inode_m *ip;

	if (file->f_fs->f_fs != EXT2)
		return -EINVAL;
	ip = file->f_inode;
	if (ip == NULL) {
		printk("ext2_read: file has no inode!\r\n");
		return -EINVAL;
	}
	if (EXT2_S_ISDIR(ip->i_ino.i_mode) || EXT2_S_ISREG(ip->i_ino.i_mode))
		return file_read(file, ip, buf, count);
	if (EXT2_S_ISBLK(ip->i_ino.i_mode))
		panic("TODO: Implement block read for ext2");
	if (EXT2_S_ISCHR(ip->i_ino.i_mode))
		return chr_devio(ip->i_ino.i_block[0], buf, count, READ);
	return -EINVAL;
}

int
ext2_write(struct file *file, void *buf, size_t count)
{
	struct ext2_inode_m *ip;

	if (file->f_fs->f_fs != EXT2)
		return -EINVAL;
	ip = file->f_inode;
	if (ip == NULL) {
		printk("ext2_write: file has no inode!\r\n");
		return -EINVAL;
	}
	if (EXT2_S_ISDIR(ip->i_ino.i_mode) || EXT2_S_ISREG(ip->i_ino.i_mode))
		file_write(file, ip, buf, count);
	if (EXT2_S_ISBLK(ip->i_ino.i_mode))
		panic("TODO: Implement block write for ext2");
	if (EXT2_S_ISCHR(ip->i_ino.i_mode))
		return chr_devio(ip->i_ino.i_block[0], buf, count, WRITE);
	return -EINVAL;
}

static ssize_t
file_read(struct file *file, struct ext2_inode_m *ip, void *buf, size_t count)
{
	off_t c = count, off, nr;
	struct buffer *bp;
	struct ext2_superblock_m *sb;
	ext2_block block;
	char *p, *s;
	ssize_t err = 0;

	sb = file->f_fs->f_super;
	p = buf;
	while (c) {
		block = ext2_bmap(ip, file->f_pos);
		if (block) {
			bp = bread(ip->i_dev, block);
			if (bp == NULL) {
				err = -EIO;
				break;
			}
		} else {
			break;
		}
		off = file->f_pos % EXT2_BLOCKSIZE(sb);
		s = bp->b_data + off;
		nr = (EXT2_BLOCKSIZE(sb) - off < c) ? EXT2_BLOCKSIZE(sb) - off: c;
		c -= nr;
		file->f_pos += nr;
		while (nr--) {
			put_ubyte(p, *s);
			p++;
			s++;
		}
		brelse(bp);
		bp = NULL;
	}
	if (bp != NULL)
		brelse(bp);
	/*
	 * We are counting on the fact the the inode waa locked
	 * in 'ext2_iget', whcih it should be.
	 */
	ip->i_ino.i_atime = CURRENT_TIME;
	ip->i_flags |= I_MODIFIED;
	mutex_unlock(&ip->i_mutex);
	if (err != 0)
		return err;
	return count - c;
}

static ssize_t
file_write(struct file *file, struct ext2_inode_m *ip, void *buf, size_t count)
{
	off_t c = count, off, nr, pos;
	struct buffer *bp;
	struct ext2_superblock_m *sb;
	ext2_block block;
	char *p, *s;
	ssize_t err = 0;

	pos = (file->f_flags & O_APPEND) ? (off_t)ip->i_ino.i_size : file->f_pos;
	sb = file->f_fs->f_super;
	p = buf;
	while (c) {
		block = ext2_create_block(ip, pos);
		if (block) {
			bp = bread(ip->i_dev, block);
			if (bp == NULL) {
				err = -EIO;
				break;
			}
		} else {
			break;
		}
		off = pos % EXT2_BLOCKSIZE(sb);
		s = bp->b_data + off;
		nr = (EXT2_BLOCKSIZE(sb) - off < c) ? EXT2_BLOCKSIZE(sb) - off : c;
		c -= nr;
		pos += nr;
		while (nr--) {
			*s = get_ubyte(p);
			p++;
			s++;
		}
		bp->b_flags |= B_DWRITE;
		brelse(bp);
		bp = NULL;
	}
	ip->i_ino.i_mtime = CURRENT_TIME;
	ip->i_ino.i_size += count - c;
	ip->i_flags |= I_MODIFIED;
	mutex_unlock(&ip->i_mutex);
	file->f_pos = pos;
	if (bp != NULL)
		brelse(bp);
	if (err != 0)
		return err;
	return count - c;
}

off_t
ext2_lseek(off_t *ptr, void *inode, off_t pos, int whence)
{
	struct ext2_inode_m *ip = inode;
	off_t tmp;

	if (EXT2_S_ISCHR(ip->i_ino.i_mode))
		/* Implementation defined, according to POSIX */
		return 0;
	if (EXT2_S_ISFIFO(ip->i_ino.i_mode) || EXT2_S_ISSOCK(ip->i_ino.i_mode))
		return -ESPIPE;
	tmp = *ptr;
	switch (whence) {
	case SEEK_SET:
		*ptr = pos;
		break;
	case SEEK_CUR:
		*ptr += pos;
		break;
	case SEEK_END:
		*ptr = ip->i_ino.i_size + pos;
		break;
	default:
		return -EINVAL;
	}
	if (*ptr < 0) {
		*ptr = tmp;
		return -EOVERFLOW;
	}
	return *ptr;
}
