/*
 * ext2/namei.c
 * Converts a path name into an inode.
 */
#include <asm/types.h>
#include <asm/uaccess.h>

#include <fs/buffer.h>
#include <fs/ext2.h>
#include <fs/fs.h>

#include <generic/errno.h>

#include <kernel/proc.h>
#include <kernel/debug.h>

static struct ext2_inode_m *find_entry(struct ext2_inode_m *dir, const char *entry);
static struct ext2_inode_m *find_entry_in_block(struct ext2_inode_m *dir, ssize_t block, const char *entry);
static struct ext2_inode_m *find_entry_direct(struct ext2_inode_m *dir, const char *entry);
static struct ext2_inode_m *find_entry_indirect(struct ext2_inode_m *dir, ssize_t block, const char *entry);
static struct ext2_inode_m *find_entry_dindirect(struct ext2_inode_m *dir, ssize_t block, const char *entry);
static struct ext2_inode_m *find_entry_tindirect(struct ext2_inode_m *dir, ssize_t block, const char *entry);
static int namei_match(const void *p1, const void *p2, size_t n);

struct ext2_inode_m *
ext2_namei(const char *path, int *error)
{
	struct ext2_inode_m *ip, *dp;
	const char *p1, *p2;

	if (current_process->root_fs->f_fs != EXT2 && current_process->root_fs->f_fs != EXT2) {
		/*
		 * Should probably panic here.
		 * Probably not the best erro code.
		 */
		*error = -EINVAL;
		return NULL;
	}
	p1 = path;
	ip = (get_ubyte(p1) == '/') ? current_process->root_inode : current_process->cwd_inode;

	if (ip == NULL)
		panic("ext2_namei: current_process has no root inode or working directory inode");
	p1++;
loop:
	printk("ino mode %x\r\n", ip->i_ino.i_mode);
	if (get_ubyte(p1) != '\0' && !(ip->i_ino.i_mode & EXT2_S_IFDIR)) {
		*error = -ENOTDIR;
		return NULL;
	}
	if (get_ubyte(p1) == '\0')
		return ip;
	while (get_ubyte(p1) == '/')
		p1++;
	p2 = p1;
	while (get_ubyte(p1) != '/' && get_ubyte(p1) != '\0')
		p1++;
	if ((dp = find_entry(ip, p2)) == NULL) {
		*error = -ENOENT;
		return NULL;
	}
	ip = dp;
	goto loop;
}

static struct ext2_inode_m *
find_entry(struct ext2_inode_m *dir, const char *entry)
{
	struct ext2_inode_m *ip;

	if ((ip = find_entry_direct(dir, entry)) != NULL) {
		printk("find_entry returning %p\r\n", ip);
		return ip;
	}
	if ((ip = find_entry_indirect(dir, dir->i_ino.i_block[EXT2_INDIRECT_BLOCK], entry)) != NULL)
		return ip;
	if ((ip = find_entry_dindirect(dir, dir->i_ino.i_block[EXT2_DINDIRECT_BLOCK], entry)) != NULL)
		return ip;
	if ((ip = find_entry_tindirect(dir, dir->i_ino.i_block[EXT2_TINDIRECT_BLOCK], entry)) != NULL)
		return ip;
	return NULL;
}

static struct ext2_inode_m *
find_entry_direct(struct ext2_inode_m *dir, const char *entry)
{
	int i;
	struct ext2_inode_m *ip;

	for (i = 0; i <= EXT2_DIRECT_BLOCKS; i++) {
		if (dir->i_ino.i_block[i] == 0)
			return NULL;
		if ((ip = find_entry_in_block(dir, dir->i_ino.i_block[i], entry)) != NULL)
			return ip;
	}
	return NULL;
}

static struct ext2_inode_m *
find_entry_indirect(struct ext2_inode_m *dir, ssize_t block, const char *entry)
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
		if ((ip = find_entry_in_block(dir, *p, entry)) != NULL)
			goto out;
	}
out:
	brelse(bp);
	return ip;
}

static struct ext2_inode_m *
find_entry_dindirect(struct ext2_inode_m *dir, ssize_t block, const char *entry)
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
		if ((ip = find_entry_indirect(dir, *p, entry)) != NULL)
			goto out;
	}
out:
	brelse(bp);
	return ip;

}

static struct ext2_inode_m *
find_entry_tindirect(struct ext2_inode_m *dir, ssize_t block, const char *entry)
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
		if ((ip = find_entry_dindirect(dir, *p, entry)) != NULL)
			goto out;
	}
out:
	brelse(bp);
	return ip;

}

static struct ext2_inode_m *
find_entry_in_block(struct ext2_inode_m *dir, ssize_t block, const char *entry)
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

	while ((flag = namei_match((const char *)d + 8, entry, d->d_name_len)) != 0) {
		if ((char *)d - bp->b_data >= EXT2_BLOCKSIZE(sb)) {
			d = NULL;
			break;
		}
		d = (struct ext2_dir_entry *)((char *)d + d->d_rec_len);
	}
	if (flag == 0)
		ret = ext2_iget(dir->i_dev, d->d_inode);
	brelse(bp);
	return ret;
}

static int
namei_match(const void *kptr, const void *uptr, size_t n)
{
	const unsigned char *a, *b;

	a = (const unsigned char *)kptr;
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
