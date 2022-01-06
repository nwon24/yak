/*
 * ext2/inode.c
 * Functions to get inodes from the inode cache.
 * A lot of this file is actually fs independent, except for the inode structure, of course.
 * Maybe it is possible to move much of this code into the fs/ subdirectory.
 */

#include <fs/buffer.h>
#include <fs/dev.h>
#include <fs/ext2.h>

#include <kernel/debug.h>
#include <kernel/proc.h>

#include <mm/vm.h>

/*
 * Just for simplicity here.
 */
#define NR_INODE_CACHE	NR_BUF_BUFFERS
#define NR_INODE_HASH	NR_BUF_HASH

#define	INODE_HASH(dev, num)	(((dev) ^ (num)) % NR_INODE_HASH)

static struct ext2_inode_m free_list;
static struct ext2_inode_m **hash_queues;

static struct ext2_inode_m *in_hash_queue(dev_t dev, ino_t num);
static void remove_from_free_list(struct ext2_inode_m *ip);
static void remove_from_hash_queue(struct ext2_inode_m *ip);
static void put_into_hash_queue(struct ext2_inode_m *ip);
static void put_into_free_list(struct ext2_inode_m *ip);
static void read_inode(struct ext2_inode_m *ip);
static void write_inode(struct ext2_inode_m *ip);

/*
 * Pretty much the same as how the buffer cache
 * is initialised.
 */
void
ext2_inodes_init(void)
{
	int i;
	struct ext2_inode_m *ip;

	hash_queues = kvmalloc(NR_INODE_HASH * sizeof(*hash_queues));
	if (hash_queues == NULL)
		panic("Unable to allocate hash table for inodes: kvamlloc returned NULL");
	for (i = 0; i < NR_BUF_HASH; i++)
		hash_queues[i] = NULL;
	free_list.i_next_free = kvmalloc(NR_INODE_CACHE * sizeof(*free_list.i_next_free));
	if (free_list.i_next_free == NULL)
		panic("Unable to allocate inode cache: kvmalloc returned NULL");
	for (ip = free_list.i_next_free; ip < free_list.i_next_free + NR_INODE_CACHE; ip++) {
		ip->i_dev = NODEV;
		if (ip == free_list.i_next_free)
			ip->i_prev_free = &free_list;
		else
			ip->i_prev_free = ip - 1;
		if (ip == free_list.i_next_free + NR_INODE_CACHE - 1)
			ip->i_next_free = &free_list;
		else
			ip->i_next_free = ip + 1;
		ip->i_next = NULL;
		ip->i_prev = NULL;
		ip->i_count = 0;
		ip->i_flags = 0;
	}
	free_list.i_prev_free = ip;
}

struct ext2_inode_m *
ext2_iget(dev_t dev, ino_t num)
{
	struct ext2_inode_m *ip;

	while (1) {
		if ((ip = in_hash_queue(dev, num)) != NULL) {
			if (ip->i_mutex == MUTEX_LOCKED) {
				sleep(ip);
				continue;
			}
			mutex_lock(&ip->i_mutex);
			remove_from_free_list(ip);
			ip->i_count++;
			return ip;
		}
		if (free_list.i_next_free == NULL)
			return NULL;
		ip = free_list.i_next_free;
		mutex_lock(&ip->i_mutex);
		remove_from_free_list(ip);
		ip->i_dev = dev;
		ip->i_num = num;
		remove_from_hash_queue(ip);
		put_into_hash_queue(ip);
		read_inode(ip);
		ip->i_count = 1;
		return ip;
	}
}

void
ext2_iput(struct ext2_inode_m *ip)
{
	if (ip->i_mutex != MUTEX_LOCKED)
		mutex_lock(&ip->i_mutex);
	if (ip->i_count-- == 0)
		panic("ext2_iput: ip->i_count = 0");
	if (ip->i_count == 0) {
		if (ip->i_ino.i_links_count == 0)
			printk("ext2_iput: TODO: ifree");
		if (ip->i_flags & I_MODIFIED)
			write_inode(ip);
		put_into_free_list(ip);
	}
	mutex_unlock(&ip->i_mutex);
}

static void
remove_from_free_list(struct ext2_inode_m *ip)
{
	if (ip->i_prev_free == NULL && ip->i_next_free == NULL)
		return;
	if (ip->i_prev_free == NULL || ip->i_next_free == NULL)
		panic("remove_from_free_list: inode free list has been corrupted");
	ip->i_prev_free->i_next_free = ip->i_next_free;
	ip->i_next_free->i_prev_free = ip->i_prev_free;
}

static void
remove_from_hash_queue(struct ext2_inode_m *ip)
{
	if (ip->i_prev == NULL && ip->i_next == NULL)
		return;
	if (ip->i_prev == NULL || ip->i_prev == NULL)
		panic("remove_from_hash_queue: inode hash queue corrupted");
	ip->i_prev->i_next = ip->i_next;
	ip->i_next->i_prev = ip->i_prev;
}

static void
put_into_hash_queue(struct ext2_inode_m *ip)
{
	struct ext2_inode_m *ip1, *ip2;

	if ((ip1 = hash_queues[INODE_HASH(ip->i_dev, ip->i_num)]) == NULL) {
		hash_queues[INODE_HASH(ip->i_dev, ip->i_num)] = ip;
		ip->i_next = ip;
		ip->i_prev = ip;
		return;
	}
	for (ip2 = ip1; ip2->i_next != ip1; ip2 = ip2->i_next);
	ip2->i_next = ip;
	ip->i_prev = ip2;
	ip->i_next = ip1;
	ip1->i_prev = ip;
}

static void
read_inode(struct ext2_inode_m *ip)
{
	struct ext2_superblock_m *sb;
	struct ext2_inode *i;
	struct ext2_blk_group_desc *bgd;
	struct buffer *bp;
	int block, index;

	/*
	 * 'get_ext2_superblock' never fails. If it does, the kernel panics.
	 * See super.c
	 */
	sb = get_ext2_superblock(ip->i_dev);
	bgd = sb->bgd_table + EXT2_BLOCK_GROUP(ip, sb);
	index = EXT2_INODE_INDEX(ip, sb);
	block = EXT2_INODE_BLOCK(index, sb);
	bp = bread(ip->i_dev, block + bgd->bg_inode_table);
	i = (struct ext2_inode *)(bp->b_data + EXT2_INODE_SIZE(sb) * (index % EXT2_INODES_PER_BLOCK(sb)));
	ip->i_ino = *i;
	brelse(bp);
}

static void
write_inode(struct ext2_inode_m *ip)
{
	struct ext2_superblock_m *sb;
	struct ext2_inode *i;
	struct buffer *bp;
	struct ext2_blk_group_desc *bgd;
	int index, block;

	sb = get_ext2_superblock(ip->i_dev);
	bgd = sb->bgd_table + EXT2_BLOCK_GROUP(ip, sb);
	index = EXT2_INODE_INDEX(ip, sb);
	block = EXT2_INODE_BLOCK(index, sb);
	bp = bread(ip->i_dev, block + bgd->bg_inode_table);
	i = (struct ext2_inode *)(bp->b_data + EXT2_INODE_SIZE(sb) * (index % EXT2_INODES_PER_BLOCK(sb)));
	*i = ip->i_ino;
	ip->i_flags &= ~I_MODIFIED;
	/* bwrite() releases the buffer */
	bwrite(bp);
}

static struct ext2_inode_m *
in_hash_queue(dev_t dev, ino_t num)
{
	struct ext2_inode_m *ip, *tmp;

	if ((ip = hash_queues[INODE_HASH(dev, num)]) == NULL)
		return NULL;
	tmp = ip;
	/*
	 * Hash queue entry is a doubly linked list, so test for coming back around
	 * rather than NULL.
	 */
	do {
		if (ip->i_dev == dev && ip->i_num == num)
			return ip;
		ip = ip->i_next;
	} while (ip != tmp);
	return NULL;
}

static void
put_into_free_list(struct ext2_inode_m *ip)
{
	if (ip->i_count)
		panic("put_into_free_list: inode reference count is not zero");
	ip->i_next_free = &free_list;
	ip->i_prev_free = free_list.i_prev_free;
	free_list.i_prev_free->i_next_free = ip;
	free_list.i_prev_free = ip;
}
