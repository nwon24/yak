/*
 * keyboard.c
 * Generic keyboard driver.
 * Tries to figure out which keyboard driver to use.
 * Right now only PS/2 keyboards are supported as there is not USB support.
 */
#include <stdint.h>

#include <asm/idt.h>
#include <asm/segment.h>
#include <asm/port_io.h>
#include <asm/pic_8259.h>

#include <drivers/acpi.h>
#include <drivers/driver.h>
#include <drivers/pci.h>
#include <drivers/ps2.h>
#include <drivers/tty.h>
#include <drivers/keyboard.h>

#include <kernel/debug.h>

#include "defkeymap.h"

#define RAW_KEYCODE(k)	((k) & 0xFF)

void irq1_handler(void);

struct keymap {
	const uint8_t *map;
	const uint8_t *shiftmap;
	const char **fnmap;
};

struct keymap loaded_key_map = {
	.map = defkeymap,
	.shiftmap = defkeymap_shift,
	.fnmap = defkeymap_fn,
};

typedef void (*k_handler)(struct kbd_packet *packet);

static void k_do_self(struct kbd_packet *packet);
static void k_do_fn(struct kbd_packet *packet);
static void k_do_mod(struct kbd_packet *packet);
static void k_do_none(struct kbd_packet *packet);

static k_handler handlers[] = {
	k_do_self, k_do_fn, k_do_mod, k_do_none
};

struct kbd_state {
	int modifiers;
	int caps_lock;
	int num_lock;
	int scroll_lock;
};

static struct kbd_state state = { 0, 0, 0, 0 };

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
	return 0;
}

static void
k_do_self(struct kbd_packet *packet)
{
	uint16_t keycode;
	const uint8_t *map;
	int c;
	char buf[3];

	if (packet->type == BREAK)
		return;
	keycode = packet->keycode;
	map = (state.modifiers & SHIFT) ? loaded_key_map.shiftmap : loaded_key_map.map;
	c = map[RAW_KEYCODE(keycode)];
	if (state.caps_lock) {
		if (c >= 'a' && c <= 'z')
			c = 'A' + (c - 'a');
		else if (c >= 'A' && c <= 'Z')
			c = 'a' + (c - 'A');
	}
	if (state.modifiers & CTRL)
		c &= 0x1F;
	if (state.modifiers & ALT) {
		buf[0] = 27;
		buf[1] = c;
		buf[2] = '\0';
	} else {
		buf[0] = c;
		buf[1] = '\0';
	}
	do_update_tty(buf);
}

static void
k_do_fn(struct kbd_packet *packet)
{
	const char **map;

	if (packet->type == BREAK)
		return;
#ifdef CONFIG_DEBUG
	if (packet->keycode == KEY_F12)
		debug_dump();
#endif
	map = loaded_key_map.fnmap;
	do_update_tty(map[RAW_KEYCODE(packet->keycode)]);
}


static void
k_do_mod(struct kbd_packet *packet)
{
	switch (packet->keycode) {
	case KEY_LSHIFT:
	case KEY_RSHIFT:
		if (packet->type == MAKE)
			state.modifiers |= SHIFT;
		else
			state.modifiers &= ~SHIFT;
		break;
	case KEY_RCTRL:
	case KEY_LCTRL:
		if (packet->type == MAKE)
			state.modifiers |= CTRL;
		else
			state.modifiers &= ~CTRL;
		break;
	case KEY_LALT:
	case KEY_RALT:
		if (packet->type == MAKE)
			state.modifiers |= ALT;
		else
			state.modifiers &= ~ALT;
		break;
	case KEY_CAPS_LOCK:
		if (packet->type == MAKE)
			state.caps_lock = 1 - state.caps_lock;
		break;
	case KEY_NUM_LOCK:
		if (packet->type == MAKE)
			state.num_lock = 1 - state.num_lock;
		break;
	}
}


static void
k_do_none(struct kbd_packet *packet)
{
	printk("[KEYBOARD] Received keycode %x that has no effect\r\n", packet->keycode);
}

void
kbd_handle_packet(struct kbd_packet *packet)
{
	int handler;

	handler = (packet->keycode >> 8) & 0xF;
	(*handlers[handler])(packet);
}
