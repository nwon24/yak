#ifndef _INTERRUPTS_H
#define _INTERRUPTS_H

#ifndef _ASSEMBLY_

#include <asm/8259_pic.h>
#include <asm/idt.h>

#include <kernel/config.h>

#ifdef _CONFIG_USE_INLINE_ASM
#define enable_intr()	__asm__("sti")
#define disable_intr()	__asm__("cli")
#else
void enable_intr(void);
void disable_intr(void);
#endif /* _CONFIG_USE_INLINE_ASM */

static inline void
interrupts_init(void)
{
	idt_init();
	pic_remap();
	enable_intr();
}

#endif /* _ASSEMBLY_ */

#endif /* _INTERRUPTS_H */
