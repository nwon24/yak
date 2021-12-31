/*
 * pci.c
 * Core PCI file.
 * See wiki.osdev.org/PCI for a general overview of PCI.
 */
#include <stdint.h>
#include <stddef.h>

#include <asm/port_io.h>

#include <kernel/debug.h>

#include <drivers/pci.h>

#define PCI_CONFIG_ADDRESS_ENABLE	(1 << 31)
/* These two are the same - just give different names depending on the context. */
#define PCI_DEVICE_NON_EXISTENT		0xFFFF
#define PCI_FUNCTION_NON_EXISTENT	0xFFFF

#define PCI_HEADER_PCI_TO_PCI		0x1
#define PCI_HEADER_PCI_TO_CARDBUS	0x2

#define PCI_DEVICE_MULTIPLE_FUNC	(1 << 7)

static uint32_t pci_read_register(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
static void pci_enumerate(void);
static void pci_check_bus(uint8_t bus);
static void pci_check_buses(void);
static void pci_check_device(uint8_t bus, uint8_t device);
static void pci_check_function(uint8_t bus, uint8_t device, uint8_t function);
static uint8_t get_secondary_bus(uint8_t bus, uint8_t device, uint8_t function);

static struct pci_entry pci_table[PCI_MAX_BUSES * PCI_MAX_DEVICES * PCI_MAX_FUNCTIONS];
static struct pci_entry *next_entry = NULL;
static int pci_entries = 0;

static inline uint16_t
get_vendor_id(uint8_t bus, uint8_t device, uint8_t function)
{
	return pci_read_register(bus, device, function, 0) & 0xFFFF;
}

static inline uint16_t
get_device_id(uint8_t bus, uint8_t device, uint8_t function)
{
	return (pci_read_register(bus, device, function, 0) >> 16) & 0xFFFF;
}

static inline uint8_t
get_header_type(uint8_t bus, uint8_t device, uint8_t function)
{
	return (pci_read_register(bus, device, function, 0xC) >> 16) & 0xFF;
}

static inline uint8_t
get_class_code(uint8_t bus, uint8_t device, uint8_t function)
{
	return pci_read_register(bus, device, function, 0x8) >> 24;
}

static inline uint8_t
get_subclass(uint8_t bus, uint8_t device, uint8_t function)
{
	return (pci_read_register(bus, device, function, 0x8) >> 16) & 0xFF;
}

static inline uint8_t
get_secondary_bus(uint8_t bus, uint8_t device, uint8_t function)
{
	return (pci_read_register(bus, device, function, 0x18) >> 8) & 0xFF;
}

static inline uint8_t
get_prog_if(uint8_t bus, uint8_t device, uint8_t function)
{
	return (pci_read_register(bus, device, function, 0x8) >> 8) & 0xFF;
}

static inline uint32_t
get_bar(uint8_t bus, uint8_t device, uint8_t function, int bar)
{
	if (bar > 5 || bar < 0)
		return 0;
	return pci_read_register(bus, device, function, 0x10 + (bar << 2));
}

static inline uint8_t
get_int_line(uint8_t bus, uint8_t device, uint8_t function)
{
	/*
	 * We should probably check the header type...
	 * We are banking on the fact that whoever calls this knows what
	 * they are doing.
	 */
	return pci_read_register(bus, device, function, 0x3C) & 0xFF;
}

/*
 * Scan the PCI buses and for each device/function that exists, put it
 * into a table. Later this should be used to build a device tree or
 * something so that drivers can be loaded.
 */
void
pci_init(void)
{
	next_entry = pci_table;
	pci_enumerate();
}

int
pci_nr_entries(void)
{
	return pci_entries;
}

static void
pci_enumerate(void)
{
	pci_check_buses();
}

static void
pci_check_function(uint8_t bus, uint8_t device, uint8_t function)
{
	uint8_t class_code, subclass, secondary_bus;
	uint16_t vendor_id;

	class_code = get_class_code(bus, device, function);
	subclass = get_subclass(bus, device, function);
	/* 0x4 subclass means PCI-to-PCI bridge. */
	if ((class_code == PCI_CLASS_CODE_BRIDGE) && (subclass == 0x4)) {
		secondary_bus = get_secondary_bus(bus, device, function);
		pci_check_bus(secondary_bus);
		return;
	}
	if ((vendor_id = get_vendor_id(bus, device, function)) != PCI_FUNCTION_NON_EXISTENT) {
		next_entry->bus = bus;
		next_entry->device = device;
		next_entry->function = function;
		next_entry->flag = 1;
		next_entry++;
		pci_entries++;
	}
}

static uint32_t
pci_read_register(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset)
{
	uint32_t out_val;

	/* Make sure the last two bits of offset are clear */
	out_val = (bus << 16) | (device << 11) | (function << 8) | (offset & 0xFC);
	out_val |= PCI_CONFIG_ADDRESS_ENABLE;
	outl(out_val, PCI_CONFIG_ADDRESS);
	return inl(PCI_CONFIG_DATA);
}

static void
pci_check_bus(uint8_t bus)
{
	uint8_t i;
	for (i = 0; i < PCI_MAX_DEVICES; i++)
		pci_check_device(bus, i);
}

static void
pci_check_device(uint8_t bus, uint8_t device)
{
	uint8_t func = 0;
	uint8_t header_type;

	if (get_vendor_id(bus, device, func) == PCI_DEVICE_NON_EXISTENT)
		return;
	pci_check_function(bus, device, func);
	header_type = get_header_type(bus, device, func);
	if (header_type & PCI_DEVICE_MULTIPLE_FUNC) {
		for (func = 1; func < PCI_MAX_FUNCTIONS; func++)
			pci_check_function(bus, device, func);
	}
}

static void
pci_check_buses(void)
{
	uint8_t func;

	if (!(get_header_type(0, 0, 0) & PCI_DEVICE_MULTIPLE_FUNC)) {
		pci_check_bus(0);
	} else {
		for (func = 0; func < PCI_MAX_FUNCTIONS; func++) {
			if (get_vendor_id(0, 0, func) != PCI_FUNCTION_NON_EXISTENT)
				break;
			pci_check_bus(func);
		}
	}
}

/*
 * Interface used by drivers.
 * Just thin wrappers.
 * Function names have 'pci_' added to the front.
 */

int
pci_find_class_code(int code, uint8_t *bus, uint8_t *device, uint8_t *function)
{
	struct pci_entry *p;
	int ret = 0;

	for (p = pci_table; p < pci_table + pci_entries; p++) {
		if (get_class_code(p->bus, p->device, p->function) == code) {
			ret = 1;
			*bus = p->bus;
			*device = p->device;
			*function = p->function;
			break;
		}
	}
	return ret;
}

uint16_t
pci_get_vendor_id(uint8_t bus, uint8_t device, uint8_t function)
{
	return get_vendor_id(bus, device, function);
}

uint16_t
pci_get_device_id(uint8_t bus, uint8_t device, uint8_t function)
{
	return get_device_id(bus, device, function);
}

uint8_t
pci_get_header_type(uint8_t bus, uint8_t device, uint8_t function)
{
	return get_header_type(bus, device, function);
}

uint8_t
pci_get_class_code(uint8_t bus, uint8_t device, uint8_t function)
{
	return get_class_code(bus, device, function);
}

uint8_t
pci_get_subclass(uint8_t bus, uint8_t device, uint8_t function)
{
	return get_subclass(bus, device, function);
}

uint8_t
pci_get_prog_if(uint8_t bus, uint8_t device, uint8_t function)
{
	return get_prog_if(bus, device, function);
}

uint32_t
pci_get_bar(uint8_t bus, uint8_t device, uint8_t function, int bar)
{
	return get_bar(bus, device, function, bar);
}

uint8_t
pci_get_int_line(uint8_t bus, uint8_t device, uint8_t function)
{
	return get_int_line(bus, device, function);
}
