/*
 * ata_init.c
 * Initialisation code for detecting ATA devices.
 */
#include <kernel/config.h>

#include <asm/idt.h>
#include <asm/irq.h>
#include <asm/segment.h>
#ifdef CONFIG_ARCH_X86
#include <asm/pic_8259.h>
#endif

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

	ata_select_drive(dev, 0, 0);
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
	/*
	 * See wiki.osdev.org/ATA_PIO_Mode or the ATA specification
	 * for the magic offsets into the 256 word block return from the
	 * IDENTIFY command.
	 */
	if (buf[0] & 1)
		return ATA_DEV_UNKNOWN;
	if (buf[83] & (1 << 10))
		printk("Drive supports LBA48 mode\r\n");
	if (!(dev->max_lba28 = buf[60] | (buf[61] << 16))) {
		printk("Get a newer drive that supports LBA.");
		return ATA_DEV_UNKNOWN;
	}
	/*
	 * For 48-bit LBA, a 32-bit kernel can only access up to 0xFFFFFFFF sectors,
	 * which is around 2 TiB.
	 * No support for 64-bit, so no '#else' bit.
	 */
#ifdef CONFIG_ARCH_X86
	dev->max_lba48_low = buf[100] | (buf[101] << 16);
	dev->max_lba48_high = buf[102] | (buf[103] << 16);
#endif /* _CONFIG_ARCH_X86 */
	/*
	 * Disable interrupts since the system begins in singletasking mode and we want to
	 * use PIO (with polling). In multitasking mode, DMA (if supported) is better.
	 */
	ata_disable_intr(dev);
	return ATA_DEV_PATA;
}

void
ata_probe(uint32_t bar0,
          uint32_t bar1,
	  uint32_t bar2,
	  uint32_t bar3,
	  uint32_t bar4,
	  int pri_irq,
	  int sec_irq)
{
	struct ata_device *dev;

	for (dev = ata_drives; dev < ata_drives + ATA_MAX_DRIVES; dev++) {
		int i = dev - ata_drives;

		if (i < ATA_DRIVES_PER_BUS)
			dev->bus = ATA_PRIMARY_BUS;
		else
			dev->bus = ATA_SECONDARY_BUS;
		if (!(i % 2)) {
			dev->drive = ATA_BUS_DRIVE1;
			ata_reset_bus(dev);
			ata_disable_intr(dev);
		} else {
			dev->drive = ATA_BUS_DRIVE2;
		}
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
		if (ata_identify(dev) == ATA_DEV_PATA) {
			printk("channel %x, drive %x\r\n", dev->bus, dev->drive);
		}
	}
	ata_pio_init();
#ifdef CONFIG_ARCH_X86
	set_idt_entry(pri_irq + IRQ_BASE, (uint32_t)irq14_handler, KERNEL_CS_SELECTOR, DPL_0, IDT_32BIT_INT_GATE);
	set_idt_entry(sec_irq + IRQ_BASE, (uint32_t)irq15_handler, KERNEL_CS_SELECTOR, DPL_0, IDT_32BIT_INT_GATE);
	/* Clear PIC2 mask */
	pic_clear_mask(0x22);
	pic_clear_mask(pri_irq + IRQ_BASE);
	pic_clear_mask(sec_irq + IRQ_BASE);
#endif
}
