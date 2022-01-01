/*
 * ata_pio.c
 * Driver for ATA in PIO mode.
 * Slower than DMA but easier to use.
 */
#include <stddef.h>

#include <asm/port_io.h>

#include <drivers/ata.h>
#include <drivers/drive.h>

#include <kernel/debug.h>

static int ata_pio_start(int drive, char *buf, size_t count, size_t lba, int rw);
static int ata_pio_poll_read(struct ata_request *req);
static int ata_pio_poll_write(struct ata_request *req);

static struct drive_driver ata_pio_driver = {
	.drive_start = ata_pio_start,
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
	if (rw == 0)
		cmd = ATA_CMD_READ_SECTORS;
	else
		cmd = ATA_CMD_WRITE_SECTORS;
	if ((req = ata_build_request(dev, lba, count, cmd, buf)) == NULL) {
		printk("TODO: Out of ata_requests. Use sleep()\r\n");
		return -1;
	}
	/*
	 * TODO: Implement IRQs for PIO mode.
	 */
	ata_start_request(req);
	if (rw == 0)
		return ata_pio_poll_read(req);
	else
		return ata_pio_poll_write(req);
	ata_finish_request(req);
}

static int
ata_pio_poll_read(struct ata_request *req)
{
	struct ata_device *dev = req->dev;

	while (req->count--) {
		ata_poll_bsy(dev);
		ata_poll_drq(dev);
		if (ata_error(dev))
			return -1;
		ata_pio_transfer(dev, req->buf, ATA_PIO_IN);
		io_delay();
		req->buf = (char *)req->buf + 512;
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
		if (ata_error(dev))
			return -1;
		ata_pio_transfer(dev, req->buf, ATA_PIO_OUT);
		io_delay();
		req->buf = (char *)req->buf + 512;
	}
	return 0;
}
