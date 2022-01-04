/*
 * ata_subr
 * Contains a few subroutines.
 */
#include <stddef.h>

#include <asm/port_io.h>

#include <drivers/ata.h>

#include <kernel/debug.h>

static struct ata_device *last_selected = NULL;

/*
 * A few helpful functions in case we need to test the bits directly.
 */
static inline int
bsy_bit(struct ata_device *dev)
{
	return ata_reg_read(dev, ATA_REG_ALT_STATUS) & ATA_STATUS_BSY;
}

static inline int
drq_bit(struct ata_device *dev)
{
	int a;
	a = ata_reg_read(dev, ATA_REG_ALT_STATUS);
	return a & ATA_STATUS_DRQ;
}

/*
 * These next two functions write to the IDE I/O registers.
 * Use this because registers may have been mapped to somewhere else,
 * and not there standard values.
 */
void
ata_reg_write(struct ata_device *dev, uint32_t val, int reg)
{
	switch (reg) {
	case ATA_REG_DEV_CTRL:
	case ATA_REG_DRIVE_ADDR:
		outb(val, dev->ctrl_base + reg);
		break;
	default:
		outb(val, dev->cmd_base + reg);
		break;
	}
}

uint8_t
ata_reg_read(struct ata_device *dev, int reg)
{
	switch (reg) {
	case ATA_REG_DEV_CTRL:
	case ATA_REG_DRIVE_ADDR:
		return inb(dev->ctrl_base + reg);
	default:
		return inb(dev->cmd_base + reg);
	}
	/* Unreachable */
	return 0;
}

/*
 * Polling functions to check when the drive is busy or read to send/receive data.
 */
void
ata_poll_bsy(struct ata_device *dev)
{
	while (bsy_bit(dev));
}

void
ata_poll_drq(struct ata_device *dev)
{
	while (!drq_bit(dev) && !ata_error(dev));
}

/*
 * Tests whether an error has occurred.
 */
int
ata_error(struct ata_device *dev)
{
	return ata_reg_read(dev, ATA_REG_ALT_STATUS) & ATA_STATUS_ERR;
}

/*
 * Select a drive by first checking the BSY and DRQ bits. They should be cleared
 * since this function can only be called if a current request is not being serviced.
 */
void
ata_select_drive(struct ata_device *dev, int include_lba, size_t lba)
{
	if (bsy_bit(dev) || drq_bit(dev)) {
		panic("ata_select_drive: bsy or drq not cleared");
	}
	/*
	 * Only return early if we are not selecting for 28-bit LBA, in which
	 * case we have to select again even if the last device was the same.
	 */
	if (last_selected == dev && !include_lba)
		return;
	last_selected = dev;
	if (include_lba)
		include_lba = ATA_DRIVE_LBA | ((lba >> 24) & 0x0F);
	if (dev->drive == ATA_BUS_DRIVE1)
		ata_reg_write(dev, 0xA0 | include_lba, ATA_REG_DRIVE);
	else
		ata_reg_write(dev, 0xA0 | ATA_DRIVE_DRV | include_lba, ATA_REG_DRIVE);
}

/*
 * PIO transfer.
 * For output, we can't use 'rep; outsw' (for x86) anyway as there should be a delay
 * between each word. The routine 'io_delay' plus the loop should be more than enough.
 */
void
ata_pio_transfer(struct ata_device *dev, void *buf, enum ata_pio_direction dir)
{
	int i;
	uint16_t *p;

	if (dir == ATA_PIO_IN) {
		rep_insw(dev->cmd_base + ATA_REG_DATA, buf, 256);
	} else if (dir == ATA_PIO_OUT) {
		p = buf;
		for (i = 0; i < 256; ++i) {
			outw(*p++, dev->cmd_base + ATA_REG_DATA);
			io_delay();
		}
	} else {
		panic("This should never happen because we are using enums");
	}
}

/*
 * Given a channel and a drive, find the associated ata_device structure.
 */
struct ata_device *
ata_find_device(unsigned int chan, unsigned int drive)
{
	struct ata_device *dev;

	if (chan >= 2 || drive >= ATA_DRIVES_PER_BUS)
		return NULL;
	for (dev = ata_drives; dev < ata_drives + ATA_MAX_DRIVES; dev++) {
		if (dev->bus == chan && dev->drive == drive)
			break;
	}
	if (dev >= ata_drives + ATA_MAX_DRIVES)
		return NULL;
	return dev;
}

/*
 * Resets the bus that the specified ATA device is on.
 */
void
ata_reset_bus(struct ata_device *dev)
{
	ata_reg_write(dev, ATA_DEV_CTRL_SRST, ATA_REG_DEV_CTRL);
	io_delay();
	ata_reg_write(dev, 0, ATA_REG_DEV_CTRL);
}

/*
 * Disables interrupts for the device.
 */
void
ata_disable_intr(struct ata_device *dev)
{
	ata_select_drive(dev, 0, 0);
	ata_reg_write(dev, ATA_DEV_CTRL_NIEN, ATA_REG_DEV_CTRL);
}

/*
 * Enables interrupts for the device.
 */
void
ata_enable_intr(struct ata_device *dev)
{
	ata_select_drive(dev, 0, 0);
	ata_reg_write(dev, 0, ATA_REG_DEV_CTRL);
}

/*
 * Should be used after each write command.
 */
void
ata_flush(struct ata_device *dev)
{
	/*
	 * Assuming this device was the last one selected.
	 * This should be the case because this is called before we finish
	 * handling a write request.
	 */
	ata_reg_write(dev, ATA_CMD_FLUSH, ATA_REG_CMD);
	ata_poll_bsy(dev);
	if (ata_error(dev))
		ata_reset_bus(dev);
}

int
ata_device_exists(int dev)
{
	if (dev >= ATA_MAX_DRIVES)
		return 0;
	return ata_drives[dev].exists;
}
