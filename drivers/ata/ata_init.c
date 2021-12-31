/*
 * ata_init.c
 * Initialisation code for detecting ATA devices.
 */
#include <kernel/config.h>

#include <asm/idt.h>
#include <asm/segment.h>

#include <drivers/ata.h>
#include <drivers/pci.h>

#include <kernel/debug.h>

void ignore_interrupt(void);

struct ata_device ata_drives[ATA_MAX_DRIVES];

static int ata_identify(struct ata_device *dev);

static int
ata_identify(struct ata_device *dev)
{
	uint16_t buf[256];

	/* Preliminary check. See "Floating Bus" at wiki.osdev.org/ATA_PIO_MODE */
	if (ata_reg_read(dev, ATA_REG_STATUS) == 0xFF)
		return ATA_DEV_UNKNOWN;

	ata_select_drive(dev, 0);
	ata_reg_write(dev, 0, ATA_REG_SEC_COUNT);
	ata_reg_write(dev, 0, ATA_REG_LBA1);
	ata_reg_write(dev, 0, ATA_REG_LBA2);
	ata_reg_write(dev, 0, ATA_REG_LBA3);
	ata_reg_write(dev, ATA_CMD_IDENTIFY, ATA_REG_CMD);

	/* SATA device set the ERR bit immediately */
	if (ata_error(dev)) {
		/*
		 * These magic values are given if a drive is SATA or ATAPI.
		 */
		if (ata_reg_read(dev, ATA_REG_LBA2) == 0x14 && ata_reg_read(dev, ATA_REG_LBA3) == 0xEB)
			return ATA_DEV_ATAPI;
		if (ata_reg_read(dev, ATA_REG_LBA2) == 0x3C && ata_reg_read(dev, ATA_REG_LBA3) == 0xC3)
			return ATA_DEV_SATA;
		return ATA_DEV_UNKNOWN;
	}
	if (ata_reg_read(dev, ATA_REG_STATUS) == 0)
		return ATA_DEV_UNKNOWN;	
	ata_poll_bsy(dev);
	if (ata_reg_read(dev, ATA_REG_LBA2) != 0 && ata_reg_read(dev, ATA_REG_LBA3) != 0)
		return ATA_DEV_UNKNOWN;
	ata_poll_drq(dev);
	if (ata_error(dev))
		return ATA_DEV_UNKNOWN;
	dev->exists = 1;
	ata_pio_transfer(dev, buf, ATA_PIO_IN);
	return ATA_DEV_PATA;
}

void
ata_probe(uint32_t bar0,
          uint32_t bar1,
	  uint32_t bar2,
	  uint32_t bar3,
	  uint32_t bar4,
	  int pri_irq,
	  int sec_irq,
	  enum ata_port_width width_pri,
	  enum ata_port_width width_sec)
{
	struct ata_device *dev;

	for (dev = ata_drives; dev < ata_drives + ATA_MAX_DRIVES; dev++) {
		int i = dev - ata_drives;

		if (i < ATA_DRIVES_PER_BUS) {
			dev->bus = ATA_PRIMARY_BUS;
			dev->port_width = width_pri;
		} else {
			dev->bus = ATA_SECONDARY_BUS;
			dev->port_width = width_sec;
		}
		if (!(i % 2))
			dev->drive = ATA_BUS_DRIVE1;
		else
			dev->drive = ATA_BUS_DRIVE2;
		switch (i) {
		case 0:
		case 1:
			dev->cmd_base = bar0;
			dev->ctrl_base = bar1; 
			break;
		case 2:
		case 3:
			dev->cmd_base = bar2;
			dev->ctrl_base = bar3;
		}
		dev->bus_master_base = bar4;
		if (ata_identify(dev) == ATA_DEV_PATA)
			printk("channel %x, drive %x\r\n", dev->bus, dev->drive);
	}
#ifdef CONFIG_ARCH_X86
	set_idt_entry(pri_irq, (uint32_t)ignore_interrupt, KERNEL_CS_SELECTOR, DPL_0, IDT_32BIT_INT_GATE);
	set_idt_entry(sec_irq, (uint32_t)ignore_interrupt, KERNEL_CS_SELECTOR, DPL_0, IDT_32BIT_INT_GATE);
#endif
}
