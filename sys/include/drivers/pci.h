#ifndef PCI_H
#define PCI_H

#include <stdint.h>

#define PCI_CONFIG_ADDRESS	0xCF8
#define PCI_CONFIG_DATA		0xCFC

/* Maximum number of buses */
#define PCI_MAX_BUSES		256
/* Maximum number of devices per bus */
#define PCI_MAX_DEVICES		32
/* Maximum number of functions per device */
#define PCI_MAX_FUNCTIONS	8

/* Lowest bit of BAR determines if it is a memory space or I/O space */
enum {
	PCI_BAR_MEM_SPACE,
	PCI_BAR_IO_SPACE,
};

struct pci_entry {
	int flag;
	int bus;
	int device;
	int function;
};

enum pci_class_codes {
	PCI_CLASS_CODE_UNCLASSIFIED,
	PCI_CLASS_CODE_MASS_STORAGE,
	PCI_CLASS_CODE_NETWORK,
	PCI_CLASS_CODE_DISPLAY,
	PCI_CLASS_CODE_MULTIMEDIA,
	PCI_CLASS_CODE_MEMORY,
	PCI_CLASS_CODE_BRIDGE,
	PCI_CLASS_CODE_SIMPLE_COM,
	PCI_CLASS_CODE_BASE_SYSTEM_PERIPHERAL,
	PCI_CLASS_CODE_INPUT_DEVICE,
	PCI_CLASS_CODE_DOCKING_STATION,
	PCI_CLASS_CODE_PROCESSOR,
	PCI_CLASS_CODE_SERIAL_BUS,
	PCI_CLASS_CODE_WIRELESS,
	PCI_CLASS_CODE_INTELLIGENT,
	PCI_CLASS_CODE_SATELLITE_COM,
	PCI_CLASS_CODE_ENCRYPTION,
	PCI_CLASS_CODE_SIGNAL_PROCESSING,
	PCI_CLASS_CODE_PROCESSING_ACCEL,
	PCI_CLASS_CODE_NON_ESSENTIAL,
	PCI_CLASS_CODE_CO_PROCESSOR = 0x3F,
	PCI_CLASS_CODE_UNASSIGNED = 0xFF
};

void pci_init(void);

int pci_find_class_code(int code, uint8_t *bus, uint8_t *device, uint8_t *function);
uint16_t pci_get_vendor_id(uint8_t bus, uint8_t device, uint8_t function);
uint16_t pci_get_device_id(uint8_t bus, uint8_t device, uint8_t function);
uint8_t pci_get_header_type(uint8_t bus, uint8_t device, uint8_t function);
uint8_t pci_get_class_code(uint8_t bus, uint8_t device, uint8_t function);
uint8_t pci_get_subclass(uint8_t bus, uint8_t device, uint8_t function);
uint8_t pci_get_prog_if(uint8_t bus, uint8_t device, uint8_t function);
uint32_t pci_get_bar(uint8_t bus, uint8_t device, uint8_t function, int bar);
uint8_t pci_get_int_line(uint8_t bus, uint8_t device, uint8_t function);
void pci_set_cmd_bit(uint8_t bus, uint8_t device, uint8_t function, int bit);

#endif /* PCI_H */
