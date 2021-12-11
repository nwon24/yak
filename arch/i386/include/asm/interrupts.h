#ifndef _INTERRUPTS_H
#define _INTERRUPTS_H

#ifndef _ASSEMBLY_

#include <asm/8259_pic.h>
#include <asm/idt.h>

void enable_intr(void);
void disable_intr(void);

static inline void
interrupts_init(void)
{
	idt_init();
	pic_remap();
	enable_intr();
}

#endif /* _ASSEMBLY_ */

#endif /* _INTERRUPTS_H */
