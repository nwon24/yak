#ifndef BUS_MASTER_H
#define BUS_MASTER_H

#include <stdint.h>

#define BUS_MASTER_CMD_PRI	0x0
#define BUS_MASTER_STATUS_PRI	0x2
#define BUS_MASTER_PRDT_PRI	0x4
#define BUS_MASTER_CMD_SEC	0x8
#define BUS_MASTER_STATUS_SEC	0xA
#define BUS_MASTER_PRDT_SEC	0xC

#define BUS_MASTER_CMD_START	1
#define BUS_MASTER_CMD_READ	(1 << 3)
#define BUS_MASTER_CMD_WRITE	0

#define BUS_MASTER_STATUS_ERR	(1 << 1)
#define BUS_MASTER_STATUS_IRQ	(1 << 2)

struct dma_prd {
	uint32_t buf;		/* Physical address of data buffer, must be 64K aligned */
	uint16_t bcount;	/* byte count */
	uint16_t reserved;	/* 0, except for the most significant bit if last entry */
}__attribute__((packed));

void bus_master_write(struct ata_device *dev, uint32_t val, int reg);
uint32_t bus_master_read(struct ata_device *dev, int reg);

#endif /* BUS_MASTER_H */
