/*
 * 8259_pic.c
 * Code to handle the PICs (Programmable Interrupt Controller)
 */

#include <stdint.h>

#include <asm/8259_pic.h>
#include <asm/port_io.h>

#define PIC_INIT	0x11
/* PIC2 Cascade identity at IRQ 2 */
#define PIC2_CASCADE	2
/* Tell PIC1 PIC2 is at IRQ 2 by setting 2nd bit (0100) */
#define PIC1_PIC2_IRQ	4

#define PIC_8086_MODE	1

#define PIC_READ_IRR	0xA
#define PIC_READ_ISR	0xB

/*
 * Mask an interrupt line by setting the PIC masks.
 * @line: interrupt line
 * Returns the mask value.
 */
uint8_t
pic_set_mask(uint8_t line)
{
	uint16_t port;
	uint8_t mask, l;

	if (line < PIC2_OFFSET) {
		port = PIC1_DATA;
		l = line - IRQ_BASE;
	} else {
		port = PIC2_DATA;
		l = line - IRQ_BASE - (PIC2_OFFSET - IRQ_BASE);
	}
	mask = inb(port) | (1 << l);
	outb(mask, port);
	return (mask);
}

/*
 * Allow an interrupt by clearing its PIC mask.
 * @line: interrupt line
 * Returns the mask value.
 */
uint8_t
pic_clear_mask(uint8_t line)
{
	uint16_t port;
	uint8_t mask, l;

	if (line < PIC2_OFFSET) {
		port = PIC1_DATA;
		l = line - IRQ_BASE;
	} else {
		port = PIC2_DATA;
		l = line - IRQ_BASE - (PIC2_OFFSET - IRQ_BASE);
	}
	mask = inb(port) & ~(1 << l);
	outb(mask, port);
	return (mask);
}

/*
 * Remap the PICs.
 * The offset for PIC1 is PIC1_OFFSET.
 * The offset for PIC2 is PIC2_OFFSET.
 * Masks off all interrupts.
 * No return value.
 */
void
pic_remap(void)
{
	outb(PIC_INIT, PIC1_CMD);
	outb(PIC_INIT, PIC2_CMD);
	outb(PIC1_OFFSET, PIC1_DATA);
	outb(PIC2_OFFSET, PIC2_DATA);
	outb(PIC1_PIC2_IRQ, PIC1_DATA);
	outb(PIC2_CASCADE, PIC2_DATA);
	outb(PIC_8086_MODE, PIC1_DATA);
	outb(PIC_8086_MODE, PIC2_DATA);
	/* Mask off all interrupts */
	outb(0xFF, PIC1_DATA);
	outb(0xFF, PIC2_DATA);
}

/*
 * Read the In-Service-Register (ISR) or the Interrupt Request Register (IRR)
 * to find out which interrupt just occurred.
 * @cmd: PIC_READ_IRR or PIC_READ_ISR
 */
static uint16_t
pic_read_reg(uint8_t cmd)
{
	if (cmd != PIC_READ_IRR && cmd != PIC_READ_ISR)
		return (0);
	outb(cmd, PIC1_CMD);
	outb(cmd, PIC2_CMD);
	return ((inb(PIC2_CMD) << 8) | inb(PIC1_CMD));
}

/* Return the value of the PICs' IRR. */
uint16_t
pic_read_irr(void)
{
	return (pic_read_reg(PIC_READ_IRR));
}

/* Return the value of the PICs' ISR */
uint16_t
pic_read_isr(void)
{
	return (pic_read_reg(PIC_READ_ISR));
}
