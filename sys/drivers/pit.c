/*
 * pit.c
 * Programmable interrupt timer driver.
 */
#include <kernel/config.h>

#ifdef CONFIG_ARCH_X86
#include <stdint.h>

#include <asm/pic_8259.h>
#include <asm/port_io.h>
#include <asm/idt.h>
#include <asm/irq.h>
#include <asm/segment.h>

#include <drivers/driver.h>
#include <drivers/timer.h>

#include <kernel/debug.h>

#define PIT_CHAN0_DATA	0x40
#define PIT_CHAN1_DATA	0x41
#define PIT_CHAN2_DATA	0x42
#define PIT_MODE_CMD	0x43

#define PIT_SQUARE_WAVE_MODE	0x36

#define PIT_IRQ		0x20

#define FREQUENCY	1193182

static void pit_irq_handler(void);
static void pit_start_timer(void);
int pit_init(void);

static struct timer_driver pit_interface = {
	.init = pit_start_timer,
	.irq_handler = pit_irq_handler
};

static inline void
pit_set_reload(uint16_t reload)
{
	outb(reload & 0xFF, PIT_CHAN0_DATA);
	outb((reload >> 8) & 0xFF, PIT_CHAN0_DATA);
}

static void
pit_start_timer(void)
{
	set_idt_entry(PIT_IRQ, (uint32_t)irq0_handler, KERNEL_CS_SELECTOR, DPL_0, IDT_32BIT_INT_GATE);
	outb(PIT_SQUARE_WAVE_MODE, PIT_MODE_CMD);
	pit_set_reload(FREQUENCY / HZ);
	pic_clear_mask(PIT_IRQ);
}

int
pit_init(void)
{
	register_timer_driver(&pit_interface);
	return 0;
}

static void
pit_irq_handler(void)
{
	/* Nothing */
}

#endif /* CONFIG_ARCH_X86 */
