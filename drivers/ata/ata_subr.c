/*
 * ata_subr
 * Contains a few subroutines.
 */
#include <stddef.h>

#include <asm/port_io.h>

#include <drivers/ata.h>

#include <kernel/debug.h>

static struct ata_device *last_selected = NULL;

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

int
ata_error(struct ata_device *dev)
{
	return ata_reg_read(dev, ATA_REG_ALT_STATUS) & ATA_STATUS_ERR;
}

void
ata_select_drive(struct ata_device *dev, int lba)
{
	if (bsy_bit(dev) || drq_bit(dev)) {
		panic("ata_select_drive: bsy or drq not cleared");
	}
	if (last_selected == dev)
		return;
	last_selected = dev;
	if (lba)
		lba = ATA_DRIVE_LBA;
	if (dev->drive == ATA_BUS_DRIVE1)
		ata_reg_write(dev, 0xA0 | lba, ATA_REG_DRIVE);
	else
		ata_reg_write(dev, 0xA0 | ATA_DRIVE_DRV | lba, ATA_REG_DRIVE);
}

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
	}
}
