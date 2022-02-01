/*
 * ps2-kbd.c
 * PS/2 Keyboard driver.
 */
#include <asm/idt.h>
#include <asm/segment.h>
#include <asm/port_io.h>
#include <asm/pic_8259.h>

#include <drivers/driver.h>
#include <drivers/ps2.h>

#include <kernel/debug.h>

static void ps2_keyboard_irq_handler(void);

static struct driver ps2_keyboard_driver = {
	.id = DRIVERS_KEYBOARD_DRIVER,
	.irq_handler = ps2_keyboard_irq_handler,
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

static void
ps2_keyboard_irq_handler(void)
{
	printk("[KBD INT] scancode %x ", inb(PS2_DATA_PORT));
}
