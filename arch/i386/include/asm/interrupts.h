#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#ifndef __ASSEMBLER__

#include <asm/pic_8259.h>
#include <asm/idt.h>

#include <kernel/config.h>

#ifdef CONFIG_USE_INLINE_ASM
#define enable_intr()	__asm__("sti")
#define disable_intr()	__asm__("cli")
#else
void enable_intr(void);
void disable_intr(void);
#endif /* CONFIG_USE_INLINE_ASM */

static inline void
interrupts_init(void)
{
	idt_init();
	pic_remap();
	enable_intr();
}

#endif /* __ASSEMBLER__ */

#endif /* INTERRUPTS_H */
