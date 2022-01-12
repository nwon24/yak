/*
 * ata_pio.c
 * Driver for ATA in PIO mode.
 * Slower than DMA but easier to use.
 */
#include <stddef.h>

#include <asm/port_io.h>
#include <asm/interrupts.h>

#include <drivers/ata.h>
#include <drivers/drive.h>

#include <fs/fs.h>

#include <kernel/debug.h>
#include <kernel/proc.h>

static int ata_pio_start(int drive, char *buf, size_t count, size_t lba, int rw);
static int ata_pio_poll_read(struct ata_request *req);
static int ata_pio_poll_write(struct ata_request *req);
static int ata_pio_start_request(struct ata_request *req);
static void ata_pio_read_intr(void);
static void ata_pio_write_intr(void);

static struct drive_driver ata_pio_driver = {
	.drive_start = ata_pio_start,
	.drive_exists = ata_device_exists,
	.drive_intr = NULL
};

void
ata_pio_init(void)
{
	drive_driver_register(&ata_pio_driver);
}

static int
ata_pio_start(int drive, char *buf, size_t count, size_t lba, int rw)
{
	struct ata_device *dev;
	struct ata_request *req;
	int cmd;

	if (drive >= ATA_MAX_DRIVES)
		return -1;
	if ((dev = ata_find_device(drive / ATA_DRIVES_PER_BUS, drive % ATA_DRIVES_PER_BUS)) == NULL) {
		printk("ata_pio_start called with non-existent drive\r\n");
		return -1;
	}
	if (rw == READ) {
		cmd = (lba > 0x0FFFFFFF) ? ATA_CMD_READ_SECTORS_EXT : ATA_CMD_READ_SECTORS;
	} else if (rw == WRITE) {
		cmd = (lba > 0x0FFFFFFF) ? ATA_CMD_WRITE_SECTORS_EXT : ATA_CMD_WRITE_SECTORS;
	} else {
		printk("ata_pio_start called with invalid cmd\r\n");
		return -1;
	}
	if ((req = ata_build_request(dev, lba, count, cmd, buf)) == NULL) {
		printk("TODO: Out of ata_requests. Use sleep()\r\n");
		return -1;
	}
	/*
	 * Use IRQs if in multitasking mode, poll otherwise.
	 * Polling is fine in singletasking mode since the CPU has nothing better to do anyway.
	 */
	if (system_is_multitasking()) {
		/* Use IRQs */
		if (ata_add_request(req)) {
			if (ata_pio_start_request(ata_current_req))
				sleep(ata_current_req);
		} else {
			sleep(req);
		}
	} else {
		ata_start_request(req);
		if (rw == 0)
			ata_pio_poll_read(req);
		else if (rw == 1)
			ata_pio_poll_write(req);
		ata_finish_request(req);
	}
	if (req->error)
		return -1;
	return 0;
}

static int
ata_pio_start_request(struct ata_request *req)
{
	ata_enable_intr(req->dev);
	ata_pio_driver.drive_intr = (req->cmd == ATA_CMD_WRITE_SECTORS || req->cmd == ATA_CMD_WRITE_SECTORS_EXT) ? ata_pio_write_intr : ata_pio_read_intr;
	ata_start_request(req);
	if (ata_error(req->dev)) {
		ata_reset_bus(req->dev);
		req->error = 1;
		return 0;
	}
	if (req->cmd == ATA_CMD_WRITE_SECTORS) {
		ata_poll_drq(req->dev);
		ata_pio_transfer(req->dev, req->buf, ATA_PIO_OUT);
		if (!--req->count) {
			ata_finish_request(req);
			ata_current_req = ata_current_req->next;
			ata_pio_driver.drive_intr = NULL;
			return 0;
		}
	}
	return 1;
}

static void
ata_pio_read_intr(void)
{
	ata_reg_read(ata_current_req->dev, ATA_REG_STATUS);
	ata_pio_transfer(ata_current_req->dev, ata_current_req->buf, ATA_PIO_IN);
	ata_current_req->buf = (char *)ata_current_req->buf + SECTOR_SIZE;
	ata_current_req->count--;
	if (ata_current_req->count == 0) {
		struct ata_request *old = ata_current_req;
		ata_finish_request(ata_current_req);
		ata_current_req = ata_current_req->next;
		if (ata_current_req != NULL) {
			if (!ata_pio_start_request(ata_current_req))
				wakeup(ata_current_req);
		} else {
			ata_pio_driver.drive_intr = NULL;
		}
		wakeup(old);
	}
}

static void
ata_pio_write_intr(void)
{
	ata_reg_read(ata_current_req->dev, ATA_REG_STATUS);
	ata_pio_transfer(ata_current_req->dev, ata_current_req->buf, ATA_PIO_OUT);
	ata_current_req->buf = (char *)ata_current_req->buf + SECTOR_SIZE;
	ata_current_req->count--;
	if (ata_current_req->count == 0) {
		struct ata_request *old = ata_current_req;
		ata_flush(ata_current_req->dev);
		ata_finish_request(ata_current_req);
		ata_current_req = ata_current_req->next;
		if (ata_current_req != NULL) {
			if (ata_pio_start_request(ata_current_req))
				wakeup(ata_current_req);
		} else {
			ata_pio_driver.drive_intr = NULL;
		}
		wakeup(old);
	}
}

static int
ata_pio_poll_read(struct ata_request *req)
{
	struct ata_device *dev = req->dev;

	while (req->count--) {
		ata_poll_bsy(dev);
		ata_poll_drq(dev);
		if (ata_error(dev)) {
			ata_reset_bus(dev);
			return -1;
		}
		ata_pio_transfer(dev, req->buf, ATA_PIO_IN);
		io_delay();
		req->buf = (char *)req->buf + SECTOR_SIZE;
	}
	return 0;
}

static int
ata_pio_poll_write(struct ata_request *req)
{
	struct ata_device *dev = req->dev;

	while (req->count--) {
		ata_poll_bsy(dev);
		ata_poll_drq(dev);
		if (ata_error(dev)) {
			ata_reset_bus(dev);
			return -1;
		}
		ata_pio_transfer(dev, req->buf, ATA_PIO_OUT);
		io_delay();
		req->buf = (char *)req->buf + SECTOR_SIZE;
	}
	ata_flush(dev);
	return 0;
}
