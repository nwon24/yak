#ifndef FS_EXT2_H
#define FS_EXT2_H

#include <stdint.h>

#include <asm/types.h>

#include <fs/fs.h>

/* Number of bytes from start of volume */
#define EXT2_SB_OFF	1024
/* Size of superblock */
#define EXT2_SB_SIZE	1024
/* Used to determine if it is actually ext2 */
#define EXT2_MAGIC	0xEF53

/* Root inode number */
#define EXT2_ROOT_INO	2

/*
 * Values in 'i_mode' field of an inode.
 * Type occupy top 4 bits (15 to 12).
 * Permissions occupy bottom 12 bits.
 */
#define EXT2_S_IFMT	0xF000	/* Top 4 bits */
#define EXT2_S_IFSOCK	0xC000	/* socket */
#define EXT2_S_IFLNK	0xA000	/* symbolic link */
#define EXT2_S_IFREG	0x8000	/* regular file */
#define EXT2_S_IFBLK	0x6000	/* block device */
#define EXT2_S_IFDIR	0x4000	/* directory */
#define EXT2_S_IFCHR	0x2000	/* character device */
#define EXT2_S_IFIFO	0x1000	/* fifo */

#define EXT2_S_ISUID	0x0800	/* Set process User ID */
#define EXT2_S_ISGID	0x0400	/* Set process Group ID */
#define EXT2_S_ISVTX	0x0200	/* sticky bit */

#define EXT2_S_IRUSR	0x0100	/* user read */
#define EXT2_S_IWUSR	0x0080	/* user write */
#define EXT2_S_IXUSR	0x0040	/* user execute */
#define EXT2_S_IRGRP	0x0020	/* group read */
#define EXT2_S_IWGRP	0x0010	/* group write */
#define EXT2_S_IXGRP	0x0008	/* group execute */
#define EXT2_S_IROTH	0x0004	/* others read */
#define EXT2_S_IWOTH	0x0002	/* others write */
#define EXT2_S_IXOTH	0x0001	/* others execute */

#define EXT2_S_IRWXU	(EXT2_S_IRUSR | EXT2_S_IWUSR | EXT2_S_IXUSR)
#define EXT2_S_IRWXG	(EXT2_S_IRGRP | EXT2_S_IWGRP | EXT2_S_IXGRP)
#define EXT2_S_IRWXO	(EXT2_S_IROTH | EXT2_S_IWOTH | EXT2_S_IXOTH)

#define EXT2_S_ISSOCK(m)	(((m) & EXT2_S_IFMT) == EXT2_S_IFSOCK)
#define EXT2_S_ISLNK(m)		(((m) & EXT2_S_IFMT) == EXT2_S_IFLNK)
#define EXT2_S_ISREG(m)		(((m) & EXT2_S_IFMT) == EXT2_S_IFREG)
#define EXT2_S_ISBLK(m)		(((m) & EXT2_S_IFMT) == EXT2_S_IFBLK)
#define EXT2_S_ISCHR(m)		(((m) & EXT2_S_IFMT) == EXT2_S_IFCHR)
#define EXT2_S_ISFIFO(m)	(((m) & EXT2_S_IFMT) == EXT2_S_IFIFO)
#define EXT2_S_ISDIR(m)		(((m) & EXT2_S_IFMT) == EXT2_S_IFDIR)

/* Inode type values in directory entries */
#define EXT2_FT_UNKNOWN		0
#define EXT2_FT_REG_FILE	1
#define EXT2_FT_DIR		2
#define EXT2_FT_CHRDEV		3
#define EXT2_FT_BLKDEV		4
#define EXT2_FT_FIFO		5
#define EXT2_FT_SOCK		6
#define EXT2_FT_SYMLINK		7

#define EXT2_DIRECT_BLOCKS	11
#define EXT2_INDIRECT_BLOCK	12
#define EXT2_DINDIRECT_BLOCK	13
#define EXT2_TINDIRECT_BLOCK	14

/* ext2 blocks are 32 bit */
typedef uint32_t ext2_block;

struct ext2_superblock {
	uint32_t s_inodes_count;
	uint32_t s_blocks_count;
	uint32_t s_r_blocks_count;
	uint32_t s_free_blocks_count;
	uint32_t s_free_inodes_count;
	uint32_t s_first_data_block;
	uint32_t s_log_block_size;
	uint32_t s_log_frag_size;
	uint32_t s_blocks_per_group;
	uint32_t s_frags_per_group;
	uint32_t s_inodes_per_group;
	uint32_t s_mtime;
	uint32_t s_wtime;
	uint16_t s_mnt_count;
	uint16_t s_max_mnt_count;
	uint16_t s_magic;
	uint16_t s_state;
	uint16_t s_errors;
	uint16_t s_minor_rev_level;
	uint32_t s_lastcheck;
	uint32_t s_checkinterval;
	uint32_t s_creator_os;
	uint32_t s_rev_level;
	uint16_t s_def_resuid;
	uint16_t s_def_resgid;
	/* Only if major version >= 1 */
	uint32_t s_first_ino;
	uint16_t s_inode_size;
	uint16_t s_block_group_nr;
	uint32_t s_feature_compat;
	uint32_t s_feature_incompat;
	uint32_t s_feature_io_compat;
	uint32_t s_uuid[4];
	char s_volume_name[16];
	char s_last_mounted[64];
	uint32_t s_algo_bitmap;
	uint8_t s_prealloc_blocks;
	uint8_t s_prealloc_dir_blocks;
	uint16_t s_unused;
	uint32_t s_journal_uuid[4];
	uint32_t s_journal_inum;
	uint32_t s_journal_dev;
	uint32_t s_last_orphan;
};

struct ext2_blk_group_desc {
	uint32_t bg_block_bitmap;
	uint32_t bg_inode_bitmap;
	uint32_t bg_inode_table;
	uint16_t bg_free_blocks_count;
	uint16_t bg_free_inodes_count;
	uint16_t bg_used_dirs_count;
	char bg_reserved[14];
};

/*
 * ext2 superblock as kept in memory.
 */
struct ext2_superblock_m {
	struct ext2_superblock sb;
	struct ext2_blk_group_desc *bgd_table;
	int nr_blk_group;
	mutex mutex;
	int modified;
};

struct ext2_inode {
	uint16_t i_mode;
	uint16_t i_uid;
	uint32_t i_size;
	uint32_t i_atime;
	uint32_t i_ctime;
	uint32_t i_mtime;
	uint32_t i_dtime;
	uint16_t i_gid;
	uint16_t i_links_count;
	uint32_t i_blocks;
	uint32_t i_flags;
	uint32_t i_osd1;
	ext2_block i_block[15];
	uint32_t i_generation;
	uint32_t i_file_acl;
	uint32_t i_dir_acl;
	uint32_t i_faddr;
	uint32_t i_osd2[3];
};

/*
 * Structure of inode in memory.
 */
struct ext2_inode_m {
	struct ext2_inode i_ino;
	mutex i_mutex;
	dev_t i_dev;
	ino_t i_num;
	int i_flags;
	int i_count;

	struct ext2_inode_m *i_prev_free;
	struct ext2_inode_m *i_next_free;
	struct ext2_inode_m *i_next;
	struct ext2_inode_m *i_prev;
};

#define EXT2_MAX_NAME_LEN	255
/*
 * Contains only the header fields, since the name
 * can be abitrarily long (up to 255)
 */
struct ext2_dir_entry {
	uint32_t d_inode;
	uint16_t d_rec_len;
	uint8_t d_name_len;
	uint8_t d_file_type;
};

#define I_MODIFIED	(1 << 0)
#define I_MOUNT		(1 << 1)

enum ext2_perm_mask {
	PERM_EXEC = 4,
	PERM_SRCH = 4,
	PERM_WRITE = 2,
	PERM_READ = 1,
};

int ext2_init(void);
struct ext2_superblock_m *get_ext2_superblock(dev_t dev);
size_t ext2_get_attribute(struct generic_filesystem *fs, enum fs_attribute_cmd cmd);

struct ext2_inode_m *ext2_iget(dev_t dev, ino_t num);
void ext2_iput(struct ext2_inode_m *ip);
void ext2_inodes_init(void);

struct ext2_inode_m *ext2_namei(const char *path, int *error, const char **base, struct ext2_inode_m **last_dir, struct buffer **last_dir_bp);

ext2_block ext2_balloc(struct ext2_inode_m *ip);
void ext2_bfree(dev_t dev, ext2_block block);
void ext2_bfree_indirect(dev_t dev, ext2_block block);
void ext2_bfree_dindirect(dev_t dev, ext2_block block);
void ext2_bfree_tindirect(dev_t dev, ext2_block block);

ino_t ext2_ialloc(dev_t dev);
void ext2_ifree(dev_t dev, ino_t num);
void ext2_itrunc(struct ext2_inode_m *ip);

ext2_block ext2_bmap(struct ext2_inode_m *ip, off_t off);
ext2_block ext2_create_block(struct ext2_inode_m *ip, off_t off);

void *ext2_open(const char *path, int flags, int mode, int *err);
int ext2_read(struct file *file, void *buf, size_t count);
int ext2_write(struct file *file, void *buf, size_t count);
int ext2_close(struct file *file);

int ext2_permission(struct ext2_inode_m *ip, enum ext2_perm_mask mask);

int ext2_sync(struct generic_filesystem *fs);
void ext2_inode_sync(void);

int ext2_add_dir_entry(struct ext2_inode_m *dir, struct ext2_inode_m *ip, const char *name, int len);

int ext2_unlink(const char *path);
int ext2_link(const char *path1, const char *path2);

int ext2_mknod(const char *path, mode_t mode, dev_t dev);

off_t ext2_lseek(off_t *ptr, void *inode, off_t offset, int whence);

int ext2_match(const void *a, const void *b, size_t c);

#define EXT2_BLOCKSIZE(s)	(1024 << (s)->sb.s_log_block_size)
#define EXT2_MTIME(s)		((s)->sb.s_mtime)
#define EXT2_WTIME(s)		((s)->sb.s_wtime)

#define EXT2_INODE_SIZE(s)	((s)->sb.s_rev_level >= 1 ? (s)->sb.s_inode_size : 128)
#define EXT2_INODES_PER_BLOCK(s)	(EXT2_BLOCKSIZE((s)) / EXT2_INODE_SIZE((s)))
#define EXT2_INODES_PER_GROUP(s)	((s)->sb.s_inodes_per_group)
#define EXT2_BLOCKS_PER_GROUP(s)	((s)->sb.s_blocks_per_group)
#define EXT2_BLOCK_GROUP(ino, s)	(((ino)->i_num - 1) / EXT2_INODES_PER_GROUP((s)))
#define EXT2_INODE_INDEX(ino, s)	(((ino)->i_num - 1) % EXT2_INODES_PER_GROUP((s)))
#define EXT2_INODE_BLOCK(ind, s)	((ind) * EXT2_INODE_SIZE((s)) / EXT2_BLOCKSIZE((s)))
#define EXT2_FIRST_FREE_INO(s)	((s)->sb.s_rev_level >= 1 ? (s)->sb.s_first_ino : 11)

#endif /* FS_EXT2_H */
