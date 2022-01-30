#ifndef MBR_H
#define MBR_H

#include <stdint.h>

#define MAX_NR_DRIVES	4
#define MAX_NR_PARTITIONS	4

#define MBR_PTE1_OFF	0x1BE
#define PTE_SIZE	16

#define MBR_MAGIC		0xAA55

struct mbr_partition {
	uint8_t attr;
	uint8_t chs_start[3];
	uint8_t type;
	uint8_t chs_end[3];
	uint32_t lba_start;
	uint32_t nr_sectors;
}__attribute__((packed));

#endif /* MBR_H */
