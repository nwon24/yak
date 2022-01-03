/*
 * ata_dma.c
 * ATA using DMA.
 */
#include <stddef.h>

#include <asm/paging.h>

#include <generic/string.h>

#include <drivers/ata.h>
#include <drivers/bus_master.h>
#include <drivers/drive.h>

#include <kernel/debug.h>
#include <kernel/proc.h>

static void ata_dma_start_request(struct ata_request *req);
static int ata_dma_start(int drive, char *buf, size_t count, size_t lba, int rw);
static void ata_dma_read_intr(void);
static void ata_dma_write_intr(void);

static struct drive_driver ata_dma_driver = {
	.drive_start = ata_dma_start,
	.drive_intr = NULL
};

static struct ata_dma_buffer ata_dma_current_buf;
/*
 * Since our buffer is contiguous in physical memory,
 * we only need one PRD in the PRDT. Makes things a lot easier.
 * Align on page size to make sure it doesn't cross a 64K boundary (unlikely anyway).
 */
static struct dma_prd ata_dma_current_prdt __attribute__((aligned(PAGE_SIZE)));

void
ata_dma_init(void)
{
	drive_driver_register(&ata_dma_driver);
}

static int
ata_dma_start(int drive, char *buf, size_t count, size_t lba, int rw)
{
	struct ata_device *dev;
	struct ata_request *req;
	int cmd;

	/*
	 * This can only be used when the system is in multitasking mode.
	 */
	if (!system_is_multitasking()) {
		printk("ata_dma_start: System is not in multitasking mode.");
		return -1;
	}
	if (drive >= ATA_MAX_DRIVES)
		return -1;
	if ((dev = ata_find_device(drive / ATA_DRIVES_PER_BUS, drive % ATA_DRIVES_PER_BUS)) == NULL) {
		printk("ata_dma_start called with non-existent drive\r\n");
		return -1;
	}
	if (rw == 0) {
		cmd = (lba >= 0x0FFFFFFF) ? ATA_CMD_READ_SECTORS_DMA_EXT : ATA_CMD_READ_SECTORS_DMA;
	} else if (rw == 1) {
		cmd = (lba >= 0x0FFFFFFF) ? ATA_CMD_WRITE_SECTORS_DMA_EXT : ATA_CMD_WRITE_SECTORS_DMA;
	} else {
		printk("ata_dma_start called with invalid cmd\r\n");
		return -1;
	}
	if ((req = ata_build_request(dev, lba, count, cmd, buf)) == NULL) {
		printk("TODO: Out of ata_requests. Use sleep()\r\n");
		return -1;
	}
	if (ata_add_request(req))
		 ata_dma_start_request(req);
	if (req->error)
		return -1;
	return 0;
}

static void
ata_dma_start_request(struct ata_request *req)
{
	int rw_bit;

	if (req->cmd == ATA_CMD_READ_SECTORS_DMA || req->cmd == ATA_CMD_READ_SECTORS_DMA_EXT) {
		rw_bit = BUS_MASTER_CMD_READ;
		ata_dma_driver.drive_intr = ata_dma_read_intr;
	} else if (req->cmd == ATA_CMD_WRITE_SECTORS_DMA || req->cmd == ATA_CMD_WRITE_SECTORS_DMA_EXT) {
		rw_bit = BUS_MASTER_CMD_WRITE;
		ata_dma_driver.drive_intr = ata_dma_write_intr;
		memmove(ata_dma_current_buf.dma_buf, req->buf, req->count * SECTOR_SIZE);
	} else {
		printk("ata_dma_start_request called with invalid command\r\n");
		return;
	}
	ata_enable_intr(req->dev);
	ata_dma_current_prdt.buf = PHYS_ADDR(ata_dma_current_buf.dma_buf);
	ata_dma_current_prdt.bcount = req->count * SECTOR_SIZE;
	ata_dma_current_prdt.reserved = 1 << 15;
	if (req->dev->drive == ATA_BUS_DRIVE1) {
		bus_master_write(req->dev, PHYS_ADDR(&ata_dma_current_prdt), BUS_MASTER_PRDT_PRI);
		bus_master_write(req->dev,
				 bus_master_read(req->dev, BUS_MASTER_STATUS_PRI) & ~BUS_MASTER_STATUS_IRQ & ~BUS_MASTER_STATUS_ERR,
				 BUS_MASTER_STATUS_PRI);
	} else {
		bus_master_write(req->dev, PHYS_ADDR(&ata_dma_current_prdt), BUS_MASTER_PRDT_SEC);
		bus_master_write(req->dev,
				 bus_master_read(req->dev, BUS_MASTER_STATUS_SEC) & ~BUS_MASTER_STATUS_IRQ & ~BUS_MASTER_STATUS_ERR,
				 BUS_MASTER_STATUS_SEC);
	}
	ata_start_request(req);
	if (req->dev->drive == ATA_BUS_DRIVE1)
		bus_master_write(req->dev, BUS_MASTER_CMD_START | rw_bit, BUS_MASTER_CMD_PRI);
	else
		bus_master_write(req->dev, BUS_MASTER_CMD_START | rw_bit, BUS_MASTER_CMD_SEC);
	if (ata_error(req->dev)) {
		req->error = 1;
		return;
	}
	sleep(req);
}

static void
ata_dma_write_intr(void)
{
	struct ata_device *dev = ata_current_req->dev;

	if (dev->drive == ATA_BUS_DRIVE1) {
		bus_master_write(dev, 0, BUS_MASTER_CMD_PRI);
		if (!(bus_master_read(dev, BUS_MASTER_STATUS_PRI) & BUS_MASTER_STATUS_IRQ))
			return;
		if (bus_master_read(dev, BUS_MASTER_STATUS_PRI) & BUS_MASTER_STATUS_ERR)
			ata_current_req->error = 1;
	} else {
		bus_master_write(dev, 0, BUS_MASTER_CMD_SEC);
		if (!(bus_master_read(dev, BUS_MASTER_STATUS_SEC) & BUS_MASTER_STATUS_IRQ))
			return;
		if (bus_master_read(dev, BUS_MASTER_STATUS_SEC) & BUS_MASTER_STATUS_ERR)
			ata_current_req->error = 1;
	}
	ata_flush(ata_current_req->dev);
	ata_finish_request(ata_current_req);
	wakeup(ata_current_req);
	if ((ata_current_req = ata_current_req->next) != NULL)
		ata_dma_start_request(ata_current_req);
	else
		ata_dma_driver.drive_intr = NULL;
}

static void
ata_dma_read_intr(void)
{
	struct ata_device *dev = ata_current_req->dev;

	if (dev->drive == ATA_BUS_DRIVE1) {
		bus_master_write(dev, 0, BUS_MASTER_CMD_PRI);
		if (!(bus_master_read(dev, BUS_MASTER_STATUS_PRI) & BUS_MASTER_STATUS_IRQ))
			return;
		if (bus_master_read(dev, BUS_MASTER_STATUS_PRI) & BUS_MASTER_STATUS_ERR) {
			ata_current_req->error = 1;
			goto wakeup_proc;
		}
	} else {
		bus_master_write(dev, 0, BUS_MASTER_CMD_SEC);
		if (!(bus_master_read(dev, BUS_MASTER_STATUS_SEC) & BUS_MASTER_STATUS_IRQ))
			return;
		if (bus_master_read(dev, BUS_MASTER_STATUS_SEC) & BUS_MASTER_STATUS_ERR) {
			ata_current_req->error = 1;
			goto wakeup_proc;
		}
	}
	memmove(ata_current_req->buf, ata_dma_current_buf.dma_buf, ata_dma_current_prdt.bcount);
wakeup_proc:
	ata_finish_request(ata_current_req);
	wakeup(ata_current_req);
	if ((ata_current_req = ata_current_req->next) != NULL)
		ata_dma_start_request(ata_current_req);
	else
		ata_dma_driver.drive_intr = NULL;
}
