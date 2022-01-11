#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#ifndef __ASSEMBLER__

#include <asm/pic_8259.h>
#include <asm/idt.h>

#include <kernel/config.h>

#ifdef CONFIG_USE_INLINE_ASM
static inline void
disable_intr(void)
{
	__asm__("pushf\n\t"
		"cli\n\t"
		"popl saved_eflags\n\t" ::);
}

static inline void
enable_intr(void)
{
	__asm__("sti");
}

/*
 * Restore interrupts flag to what it was before last 'disable_intr'
 */
static inline void
restore_eflags(void)
{
	__asm__("pushl saved_eflags\n\t"
		"popf" : :);
}
#else
void enable_intr(void);
void disable_intr(void);
void restore_eflags(void);
#endif /* CONFIG_USE_INLINE_ASM */

#define restore_intr_state	restore_eflags

static inline void
interrupts_init(void)
{
	idt_init();
	pic_remap();
	enable_intr();
}

#endif /* __ASSEMBLER__ */

#endif /* INTERRUPTS_H */
