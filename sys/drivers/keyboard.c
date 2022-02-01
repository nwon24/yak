/*
 * keyboard.c
 * Generic keyboard driver.
 * Tries to figure out which keyboard driver to use.
 * Right now only PS/2 keyboards are supported as there is not USB support.
 */
#include <stdint.h>

#include <drivers/acpi.h>
#include <drivers/driver.h>
#include <drivers/pci.h>
#include <drivers/ps2.h>
#include <drivers/tty.h>

#include <kernel/debug.h>

#include "defkeymap.h"

#define RAW_KEYCODE(k)	((k) & 0xFF)

uint8_t *loadedkeymap;

typedef void (*k_handler)(uint16_t keycode);

static void k_do_self(uint16_t keycode);
static void k_do_fn(uint16_t keycode);
static void k_do_mod(uint16_t keycode);
static void k_do_none(uint16_t keycode);

static k_handler handlers[] = {
	k_do_self, k_do_fn, k_do_mod, k_do_none
};

int
keyboard_init(void)
{
	uint8_t bus, dev, func;
	int flag;

	if (pci_find_class_code(PCI_CLASS_CODE_SERIAL_BUS, &bus, &dev, &func)) {
		printk("WARNING: Found USB controller with bus %d, device %d, function %d. There is no USB support - a USB keyboard will not be of any use!", bus, dev, func);
	}
	if ((flag = get_acpi_boot_arch_flags()) != ACPI_1_0) {
		if (!(flag & 2))
			panic("No keyboard controller found");
	}
	/*
	 * Now we know a PS/2 controller exists
	 */
	ps2_init();
	if (ps2_kbd_init() < 0)
		panic("Unable to initialise PS/2 keyboard");
	loadedkeymap = defkeymap;
	return 0;
}

static void
k_do_self(uint16_t keycode)
{
	do_update_tty(loadedkeymap[RAW_KEYCODE(keycode)]);
}

static void
k_do_fn(uint16_t keycode)
{
}


static void
k_do_mod(uint16_t keycode)
{
}


static void
k_do_none(uint16_t keycode)
{
}

void
kbd_keycode(uint16_t keycode)
{
	int handler;

	handler = (keycode >> 8) & 0xF;
	(*handlers[handler])(keycode);
}
