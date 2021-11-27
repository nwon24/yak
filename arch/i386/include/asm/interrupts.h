#ifndef _INTERRUPTS_H
#define _INTERRUPTS_H

#define ENABLE_INTERRUPTS	__asm__("sti")
#define DISABLE_INTERRUPTS	__asm__("cli")
#ifndef _ASSEMBLY_

#include <asm/8259_pic.h>
#include <asm/idt.h>

static inline void
interrupts_init(void)
{
	idt_init();
	pic_remap();
	ENABLE_INTERRUPTS;
}

#endif /* _ASSEMBLY_ */

#endif /* _INTERRUPTS_H */
