#ifndef IDT_H
#define IDT_H

#define IDT_32BIT_INT_GATE	0xE
#define IDT_32BIT_TRAP_GATE	0xF

#define IDT_ENTRY_PRESENT	0x80

#define IDT_ENTRIES		256
#define IDT_LIMIT		((IDT_ENTRIES << 3) - 1)

#ifndef __ASSEMBLER__

#include <stdint.h>

#include <asm/paging.h>

#include <kernel/config.h>

void set_idt_entry(int n, uint32_t offset, uint16_t selector, uint8_t dpl, uint8_t type);
void idt_init(void);

#ifdef CONFIG_USE_INLINE_ASM
static inline void
load_idt(void *idt_ptr)
{
	struct {
		uint16_t limit;
		uint32_t base;
	}__attribute__((packed)) idt_desc = { IDT_LIMIT, (uint32_t)idt_ptr };
	__asm__ volatile("lidt %0" : : "m" (idt_desc));
}
#else
void load_idt(void *idt_ptr);
#endif /* CONFIG_USE_INLINE_ASM */

#endif /* __ASSEMBLER__ */

#endif /* IDT_H */
