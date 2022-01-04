#ifndef FS_EXT2_H
#define FS_EXT2_H

#include <stdint.h>

#include <asm/types.h>

#include <fs/fs.h>

/* Number of bytes from start of volume */
#define EXT2_SB_OFF	1024

#define EXT2_MAGIC	0xEF53

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

/*
 * ext2 superblock as kept in memory.
 */
struct ext2_superblock_m {
	struct ext2_superblock sb;
	mutex mutex;
	int modified;
};

int ext2_init(void);
struct ext2_superblock *get_ext2_superblock(struct generic_filesystem *fs);
size_t ext2_get_attribute(struct generic_filesystem *fs, enum fs_attribute_cmd cmd);

#define EXT2_BLOCKSIZE(dev)	(1024 << (get_ext2_superblock((dev)))->s_log_block_size)
#define EXT2_MTIME(dev)		(get_ext2_superblock((dev))->s_mtime)
#define EXT2_WTIME(dev)		(get_ext2_superblock((dev))->s_wtime)

#endif /* FS_EXT2_H */
