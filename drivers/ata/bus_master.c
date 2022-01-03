/*
 * bus_master.c
 * Driver for bus master ATA controllers.
 */
#include <asm/port_io.h>

#include <drivers/ata.h>
#include <drivers/bus_master.h>
#include <drivers/pci.h>

#include <kernel/debug.h>

void
bus_master_write(struct ata_device *dev, uint32_t val, int reg)
{
	if (!dev->bus_master_base) {
		printk("bus_master_write called for a device that doesn't suppor it.");
		return;
	}
	if (dev->bus_master_type == PCI_BAR_MEM_SPACE) {
		if (reg != BUS_MASTER_PRDT_PRI || reg != BUS_MASTER_PRDT_SEC)
			*(uint8_t *)(dev->bus_master_base + reg) = (uint8_t)val;
		else
			*(uint32_t *)(dev->bus_master_base + reg) = val;
	} else {
		if (reg != BUS_MASTER_PRDT_PRI && reg != BUS_MASTER_PRDT_SEC)
			outb(val, dev->bus_master_base + reg);
		else
			outl(val, dev->bus_master_base + reg);
	}
}

uint32_t
bus_master_read(struct ata_device *dev, int reg)
{
	if (!dev->bus_master_base) {
		printk("bus_master_read called for a device that doesn't suppor it.");
		return 0;
	}
	if (dev->bus_master_type == PCI_BAR_MEM_SPACE) {
		if (reg != BUS_MASTER_PRDT_PRI || reg != BUS_MASTER_PRDT_SEC)
			return (uint32_t)(*(uint8_t *)(dev->bus_master_base + reg));
		else
			return *(uint32_t *)(dev->bus_master_base + reg);
	} else {
		if (reg != BUS_MASTER_PRDT_PRI && reg != BUS_MASTER_PRDT_SEC)
			return inb(dev->bus_master_base + reg);
		else
			return inl(dev->bus_master_base + reg);
	}
}
