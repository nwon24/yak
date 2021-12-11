/*
 * idt.c
 * Code to add entries to and initialise the IDT (Interrupt Descriptor Table).
 */

#include <asm/idt.h>
#include <asm/interrupts.h>
#include <asm/segment.h>

void exceptions_init(void);

struct idt_entry {
	uint16_t offset_low;
	uint16_t segment_selector;
	uint8_t zero;
	uint8_t type_attr;
	uint16_t offset_high;
}__attribute__((packed));

static struct idt_entry idt[IDT_ENTRIES]__attribute__((aligned(16)));

void ignore_interrupt(void);

/*
 * Put an entry into the IDT.
 * @n: number of entry.
 * @offset: address of interrupt handler.
 * @selector: code segment selector.
 * @type: Interrupt or trap gate.
 */
void
set_idt_entry(int n, uint32_t offset, uint16_t selector, uint8_t dpl, uint8_t type)
{
	struct idt_entry *ip;

	if ((ip = &idt[n]) >= &idt[IDT_ENTRIES])
		return;
	ip->offset_low = offset & 0xFFFF;
	ip->segment_selector = selector;
	ip->zero = 0;
	ip->type_attr = type | IDT_ENTRY_PRESENT | (dpl << 5);
	ip->offset_high = (offset >> 16) & 0xFFFF;
}

/*
 * Initialise the IDT by putting all entries pointing to 'ignore_interrupt'.
 */
void
idt_init(void)
{
	int i;

	for (i = IRQ_BASE; i < IDT_ENTRIES; i++)
		set_idt_entry(i, (uint32_t)ignore_interrupt, KERNEL_CS_SELECTOR, DPL_0, IDT_32BIT_INT_GATE);
	exceptions_init();
	load_idt((void *)idt);
}
