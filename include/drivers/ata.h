#ifndef ATA_H
#define ATA_H

#include <stdint.h>
#include <stddef.h>

#include <kernel/config.h>

enum ata_bus {
	ATA_PRIMARY_BUS,
	ATA_SECONDARY_BUS,
};

enum ata_drive {
	ATA_BUS_DRIVE1,
	ATA_BUS_DRIVE2
};

/* Standard I/O Ports for IDE controller */
#define ATA_PRI_CMD_BASE	0x1F0
#define ATA_PRI_CTRL_BASE	0x3F6
#define ATA_SEC_CMD_BASE	0x170
#define ATA_SEC_CTRL_BASE	0x376

/* Offsets from the command base */
#define ATA_REG_DATA		0
#define ATA_REG_ERR		1
#define ATA_REG_FEAT		1
#define ATA_REG_SEC_COUNT	2
#define ATA_REG_LBA1		3
#define ATA_REG_LBA2		4
#define ATA_REG_LBA3		5
#define ATA_REG_DRIVE		6
#define ATA_REG_STATUS		7
#define ATA_REG_CMD		7

/* Offsets from control base */
#define ATA_REG_ALT_STATUS	0
#define ATA_REG_DEV_CTRL	0
#define ATA_REG_DRIVE_ADDR	1

/* Errors */
#define ATA_ERR_AMNF		(1 << 0)	/* Address mark not found */
#define ATA_ERR_TKZNF		(1 << 1)	/* Track zero not found. */
#define ATA_ERR_ABRT		(1 << 2)	/* Aborted command */
#define ATA_ERR_MCR		(1 << 3)	/* Media change request */
#define ATA_ERR_IDNF		(1 << 4)	/* ID not found */
#define ATA_ERR_MC		(1 << 5)	/* Media changed */
#define ATA_ERR_UNC		(1 << 6)	/* Uncorrectable data error */
#define ATA_ERR_BBK		(1 << 7)	/* Bad block detected */

/* ATA_REG_DRIVE */
#define ATA_DRIVE_DRV		(1 << 4)
#define ATA_DRIVE_LBA		(1 << 6)

/* ATA_REG_STATUS */
#define ATA_STATUS_ERR		(1 << 0)	/* An error has occured */
#define ATA_STATUS_IDX		(1 << 1)	/* Always 0 */
#define ATA_STATUS_CORR		(1 << 2)	/* Corrected data. Always 0 */
#define ATA_STATUS_DRQ		(1 << 3)	/* Set when drive is ready to accept/send data (PIO) */
#define ATA_STATUS_SRV		(1 << 4)	/* Overlapped Mode Service Request (??) */
#define ATA_STATUS_DF		(1 << 5)	/* Drive Fault Error */
#define ATA_STATUS_RDY		(1 << 6)	/* Clear if error or drive is spun down */
#define ATA_STATUS_BSY		(1 << 7)	/* Drive is preparing to send/receive data */

/* ATA_REG_DEV_CTRL */
#define ATA_DEV_CTRL_NIEN	(1 << 1)	/* Set to stop interruts */
#define ATA_DEV_CTRL_SRST	(1 << 2)	/* Set and clear to do reset */
#define ATA_DEV_CTRL_HOB	(1 << 7)	/* Used to get Higher Order Byte of last LBA48 */

/* ATA_REG_DRIVE_ADDR */
#define ATA_DRIVE_ADDR_DS0	(1 << 0)	/* Clears when drive 0 selected */
#define ATA_DRIVE_ADDR_DS1	(1 << 1)	/* Clears when drive 1 selected */
#define ATA_DRIVE_ADDR_WTG	(1 << 6)	/* Write gate */

#define ATA_DRIVES_PER_BUS	2
#define ATA_MAX_DRIVES		4		/* Two buses, two drives on each bus */

#define ATA_MAX_REQUESTS	64

/* Commands */
#define ATA_CMD_READ_SECTORS	0x20
#define ATA_CMD_WRITE_SECTORS	0x30
#define ATA_CMD_READ_SECTORS_EXT	0x24
#define ATA_CMD_WRITE_SECTORS_EXT	0x34
#define ATA_CMD_READ_SECTORS_DMA	0xC8
#define ATA_CMD_WRITE_SECTORS_DMA	0xCA
#define ATA_CMD_READ_SECTORS_DMA_EXT	0x25
#define ATA_CMD_WRITE_SECTORS_DMA_EXT	0x35
#define ATA_CMD_IDENTIFY	0xEC
#define ATA_CMD_FLUSH		0xE7

#define ATA_PRI_IRQ		0xE
#define ATA_SEC_IRQ		0xF

enum ata_device_type {
	ATA_DEV_PATA,
	ATA_DEV_SATA,
	ATA_DEV_ATAPI,
	ATA_DEV_UNKNOWN,
};

enum ata_pio_direction {
	ATA_PIO_IN,
	ATA_PIO_OUT,
};

struct ata_device {
	enum ata_bus bus;
	enum ata_drive drive;

	int exists;

	uint32_t cmd_base;
	uint32_t ctrl_base;
	uint32_t bus_master_base;
	/* Type of bus master registers - memory or I/O? */
	int bus_master_type;

	uint32_t max_lba28;
#ifdef CONFIG_ARCH_X86
	uint32_t max_lba48_low;
	uint32_t max_lba48_high;
#endif /* CONFIG_ARCH_X86 */
};

struct ata_request {
	struct ata_device *dev;		/* ATA device */

	int cmd;			/* ATA command */
	size_t count;			/* Number of sectors to read/write */
	size_t lba;			/* LBA */
	void *buf;			/* Buffer to r/w from */
	int error;			/* Has there been an error */

	struct ata_request *next;	/* Next request in queue */
};

/*
 * ATA DMA is a bit of a pain since buffers and the PRDT have to be aligned
 * on 64 KiB boundaries. This is the easiest way, even though fiddling around
 * with page tables and page frames might be better.
 */
#define ATA_DMA_ALIGN	0x10000

struct ata_dma_buffer {
	char dma_buf[ATA_DMA_ALIGN]__attribute__((aligned(ATA_DMA_ALIGN)));
};

extern struct ata_device ata_drives[];
extern struct ata_request *ata_current_req;

void ata_reg_write(struct ata_device *dev, uint32_t val, int reg);
uint8_t ata_reg_read(struct ata_device *dev, int reg);
void ata_poll_bsy(struct ata_device *dev);
void ata_poll_drq(struct ata_device *dev);
void ata_select_drive(struct ata_device *dev, int include_lba, size_t lba);
int ata_error(struct ata_device *dev);
void ata_pio_transfer(struct ata_device *dev, void *buf, enum ata_pio_direction dir);
void ata_probe(uint32_t bar0, uint32_t bar1, uint32_t bar2, uint32_t bar3, uint32_t bar4, int bar4_type, int pri_irq, int sec_irq);
int ata_add_request(struct ata_request *req);
struct ata_request *ata_build_request(struct ata_device *dev, size_t lba, size_t count, int cmd, char *buf);
struct ata_device *ata_find_device(unsigned int chan, unsigned int drive);
void ata_start_request(struct ata_request *req);
void ata_finish_request(struct ata_request *req);
void ata_reset_bus(struct ata_device *dev);
void ata_enable_intr(struct ata_device *dev);
void ata_disable_intr(struct ata_device *dev);
void ata_pio_init(void);
void ata_dma_init(void);
void ata_flush(struct ata_device *dev);

#endif /* ATA_H */
