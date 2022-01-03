/*
 * drive.c
 * Contains the device independent disk driver.
 * The only common word between SSD and HDD is 'drive'.
 */

#include <drivers/ata.h>
#include <drivers/pci.h>
#include <drivers/drive.h>
#include <drivers/driver.h>

#include <kernel/debug.h>

struct drive_driver *current_driver;
static void drive_irq_handler(void);

static struct driver drive_driver = {
	.irq = 1,
	.id = DRIVERS_DISK_DRIVER,
	.irq_handler = drive_irq_handler,
};

/*
 * Mass Storage Controller (MSD) subclass values.
 */
enum msd_subclass {
	SCSI_BUS_CTRL,		/* SCSI Bus Controller */
	IDE_CTRL,		/* IDE controller */
	FLOPPY_DISK_CTRL,	/* Floppy disk controller */
	IPI_BUS_CTRL,		/* IPI Bus Controller (??) */
	RAID_CTRL,		/* RAID controller */
	ATA_CTRL,		/* ATA Controller with ADMA interface */
	SATA_CTRL,		/* Serial ATA Controler */
	SCSI_CTRL,		/* Serial Attached SCSI Controller */
	NON_VOLATILE_MEM_CTRL,	/* Non-volatile memory controller */
	OTHER = 0x80		/* Vendor specific */
};

int
drive_init(void)
{
	uint8_t bus, dev, func, prog_if;
	uint32_t bar0, bar1, bar2, bar3, bar4;
	int pri_irq, sec_irq, bar4_type;

	if (!pci_find_class_code(PCI_CLASS_CODE_MASS_STORAGE, &bus, &dev, &func))
		panic("No mass storage device found.");
	if (pci_get_subclass(bus, dev, func) != IDE_CTRL)
		panic("Currently do not have drivers for anything other than IDE controllers.");
	pri_irq = ATA_PRI_IRQ;
	sec_irq = ATA_SEC_IRQ;
	prog_if = pci_get_prog_if(bus, dev, func);
	if (prog_if & (1 << 0)) {
		/* Primary channel is in PCI native mode */
		bar0 = pci_get_bar(bus, dev, func, 0);
		bar0 &= 0xFFFFFFFC;
		bar1 = pci_get_bar(bus, dev, func, 1) + 2;
		bar1 &= 0xFFFFFFFC;
		pri_irq = sec_irq = pci_get_int_line(bus, dev, func);
	} else {
		bar0 = ATA_PRI_CMD_BASE;
		bar1 = ATA_PRI_CTRL_BASE;
	}
	if (prog_if & (1 << 2)) {
		/* Secondary channel is in PCI native mode */
		bar2 = pci_get_bar(bus, dev, func, 2);
		bar2 &= 0xFFFFFFFC;
		bar3 = pci_get_bar(bus, dev, func, 3) + 2;
		bar3 &= 0xFFFFFFFC;
		pri_irq = sec_irq = pci_get_int_line(bus, dev, func);
	} else {
		bar2 = ATA_SEC_CMD_BASE;
		bar3 = ATA_SEC_CTRL_BASE;
	}
	if (prog_if & (1 << 7)) {
		/* Bus master IDE controller */
		bar4 = pci_get_bar(bus, dev, func, 4);

		/*
		 * According to wiki.osdev.org/ATA/ATAPI_using_DMA,
		 * the Bus Master registers could be memmory mapped, so we need to check for that.
		 */
		if (bar4 & PCI_BAR_IO_SPACE) {
			bar4 &= 0xFFFFFFFC;
			bar4_type = PCI_BAR_IO_SPACE;
		} else {
			bar4 &= 0xFFFFFFF0;
			bar4_type = PCI_BAR_MEM_SPACE;
		}
#ifdef CONFIG_ARCH_X86
		if ((bar4 & (3 << 1)) == 0x2) {
			/* Address is 64-bits wide. We only have 32-bit address space here. */
			printk("Busmaster BAR is 64-bits. No DMA\r\n");
			bar4 = 0;
		}
		/* Set Bus Master bit of PCI command register */
		if (bar4)
			pci_set_cmd_bit(bus, dev, func, 2);
#endif /* CONFIG_ARCH_X86 */
	} else {
		bar4_type = 0;
		bar4 = 0;
	}
	ata_probe(bar0, bar1, bar2, bar3, bar4, bar4_type, pri_irq, sec_irq);
	register_driver(&drive_driver);
	return 0;
}

static void
drive_irq_handler(void)
{
	if (current_driver->drive_intr != NULL)
		current_driver->drive_intr();
}

void
drive_driver_register(struct drive_driver *drv)
{
	current_driver = drv;
}
