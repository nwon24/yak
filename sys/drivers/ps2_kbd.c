/*
 * ps2-kbd.c
 * PS/2 Keyboard driver.
 */
#include <stdint.h>

#include <asm/idt.h>
#include <asm/segment.h>
#include <asm/port_io.h>
#include <asm/pic_8259.h>

#include <drivers/driver.h>
#include <drivers/ps2.h>
#include <drivers/keycode.h>
#include <drivers/keyboard.h>

#include <kernel/debug.h>

#define SET_1_ESCAPE	0xE0
#define SET_1_PAUSE_ESCAPE	0xE1
#define SET_1_LSHIFT_MAKE	0x2A
#define SET_1_LSHIFT_BREAK	0xAA

enum kbd_state {
	WAITING_FOR_SCAN,
	WAITING_FOR_SECOND,
	WAITING_FOR_THIRD,
	WAITING_FOR_FOURTH,
	WAITING_FOR_FIFTH,
	WAITING_FOR_SIXTH,
};

static int state = WAITING_FOR_SCAN;
static int pause_flag = 0;

static void ps2_kbd_irq(void);
static uint16_t get_keycode_set1(int scan, int *type);

static uint16_t (*get_keycode)(int scan, int *type);

static uint16_t scancode_set1[] = {
	KEY_NONE, KEY_ESC, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_0,
	KEY_MINUS, KEY_EQUALS, KEY_BACKSPACE, KEY_TAB, KEY_Q, KEY_W, KEY_E, KEY_R, KEY_T, KEY_Y, KEY_U,
	KEY_I, KEY_O, KEY_P, KEY_LSQR_BRACKET, KEY_RSQR_BRACKET, KEY_ENTER, KEY_LCTRL, KEY_A, KEY_S, KEY_D,
	KEY_F, KEY_G, KEY_H, KEY_J, KEY_K, KEY_L, KEY_SEMI_COLON, KEY_QUOTE, KEY_BACK_TICK, KEY_LSHIFT, KEY_BACKSLASH,
	KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B, KEY_N, KEY_M, KEY_COMMA, KEY_DOT, KEY_SLASH, KEY_RSHIFT, KEY_PAD_STAR,
	KEY_LALT, KEY_SPACE, KEY_CAPS_LOCK, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9,
	KEY_F10, KEY_NUM_LOCK, KEY_SCROLL_LOCK, KEY_PAD_7, KEY_PAD_8, KEY_PAD_9, KEY_PAD_MINUS, KEY_PAD_4, KEY_PAD_5,
	KEY_PAD_6, KEY_PAD_PLUS, KEY_PAD_1, KEY_PAD_2, KEY_PAD_3, KEY_PAD_0, KEY_PAD_DOT, KEY_NONE, KEY_NONE, KEY_NONE,
	KEY_F11, KEY_F12
};

/*
 * Prefixed with 0xE0.
 */
static uint16_t scancode_set1_ext[] = {
	KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE,
	KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_MM_PREVIOUS, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE,
	KEY_NONE, KEY_MM_NEXT, KEY_NONE, KEY_NONE, KEY_PAD_ENTER, KEY_RCTRL, KEY_NONE, KEY_NONE, KEY_MM_MUTE, KEY_MM_CALCULATOR,
	KEY_MM_PLAY, KEY_NONE, KEY_MM_STOP, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE,
	KEY_MM_DOWN, KEY_NONE, KEY_MM_UP, KEY_NONE, KEY_MM_WWW_HOME, KEY_NONE, KEY_NONE, KEY_PAD_SLASH, KEY_NONE, KEY_NONE, KEY_RALT,
	KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE,
	KEY_NONE, KEY_NONE, KEY_HOME, KEY_ARROW_UP, KEY_PAGE_UP, KEY_NONE, KEY_ARROW_LEFT, KEY_NONE, KEY_ARROW_RIGHT, KEY_NONE,
	KEY_END, KEY_ARROW_DOWN, KEY_PAGE_UP, KEY_INSERT, KEY_DELETE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE,
	KEY_NONE, KEY_LSUPER, KEY_RSUPER, KEY_APPS, KEY_ACPI_POWER, KEY_ACPI_SLEEP, KEY_NONE, KEY_NONE, KEY_NONE, KEY_ACPI_WAKE,
	KEY_NONE, KEY_MM_WWW_SEARCH, KEY_MM_WWW_FAVORITES, KEY_MM_WWW_REFRESH, KEY_MM_WWW_STOP, KEY_MM_WWW_FORWARD, KEY_MM_WWW_BACK, KEY_MM_WWW_MY_COMP,
	KEY_MM_WWW_EMAIL, KEY_MM_WWW_MEDIA_SEL
};

static struct driver ps2_keyboard_driver = {
	.id = DRIVERS_KEYBOARD_DRIVER,
	.irq_handler = ps2_kbd_irq,
	.irq = 1,
};

enum ps2_channel kbd_channel;

void irq1_handler(void);
void irq12_handler(void);

int
ps2_kbd_init(void)
{
	enum ps2_channel chan = PS2_FIRST_PORT;
	int timeout;

repeat:
	timeout = 10000;
	if (ps2_dev_send_wait(chan, PS2_DEV_CMD_DISABLE_SCAN) < 0)
		return -1;
	if (ps2_dev_send_wait(chan, PS2_DEV_CMD_IDENTIFY) == 0) {
		while (!(inb(PS2_STAT_REG) & PS2_STAT_OUT_FULL)
		       && timeout--);
		if (timeout == 0)
			return -1;
		if (inb(PS2_DATA_PORT) != PS2_MF2_KBD)
			return -1;
	} else {
		if (chan == PS2_SEC_PORT)
			return -1;
		chan = PS2_SEC_PORT;
		goto repeat;
	}
	kbd_channel = chan;
	if (ps2_dev_send_wait(chan, PS2_DEV_CMD_RESET) < 0)
		return -1;
	if (ps2_dev_send_wait(chan, PS2_DEV_CMD_ENABLE_SCAN) < 0)
		return -1;
	get_keycode = get_keycode_set1;
	register_driver(&ps2_keyboard_driver);
	if (chan == PS2_FIRST_PORT) {
		set_idt_entry(PS2_FIRST_IRQ, (uint32_t)irq1_handler, KERNEL_CS_SELECTOR, DPL_0, IDT_32BIT_INT_GATE);
		pic_clear_mask(PS2_FIRST_IRQ);
	} else {
		set_idt_entry(PS2_SEC_IRQ, (uint32_t)irq12_handler, KERNEL_CS_SELECTOR, DPL_0, IDT_32BIT_INT_GATE);
		pic_clear_mask(PS2_SEC_IRQ);
	}
	return 0;
}

static uint16_t
get_keycode_set1(int scan, int *type)
{
	if (scan > 0xE1)
		return KEY_NONE;
	if (pause_flag && state == WAITING_FOR_SIXTH) {
		*type = MAKE;
		return KEY_PAUSE_BREAK;
	}
	if (scan == 0xE1) {
		pause_flag = 1;
		state++;
		return KEY_NONE;
	}
	if (scan == SET_1_ESCAPE) {
		state++;
		return KEY_NONE;
	}
	if (scan == SET_1_LSHIFT_MAKE && state == WAITING_FOR_SECOND) {
		state++;
		return KEY_NONE;
	}
	if (scan == 0x37 && state == WAITING_FOR_FOURTH) {
		*type = MAKE;
		return KEY_PRINT_SCREEN;
	}
	if (scan == 0xB7 && state == WAITING_FOR_SECOND) {
		state++;
		return KEY_NONE;
	}
	if (scan == SET_1_LSHIFT_BREAK && state == WAITING_FOR_FOURTH) {
		*type = BREAK;
		return KEY_PRINT_SCREEN;
	}
	if (scan & 0x80) {
		*type = BREAK;
		scan &= ~0x80;
	} else {
		*type = MAKE;
	}
	return (state == WAITING_FOR_SCAN) ? scancode_set1[scan] : scancode_set1_ext[scan];
}

static void
ps2_kbd_irq(void)
{
	int scan;
	struct kbd_packet packet;

	scan = inb(PS2_DATA_PORT);
	packet.keycode = (*get_keycode)(scan, &packet.type);
	if (packet.keycode == KEY_NONE)
		return;
	kbd_handle_packet(&packet);
}
