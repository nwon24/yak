#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#ifndef __ASSEMBLER__

#include <asm/cpu_state.h>
#include <asm/pic_8259.h>
#include <asm/idt.h>

#include <kernel/config.h>

#ifdef CONFIG_USE_INLINE_ASM
static inline void
disable_intr(void)
{
	__asm__("pushf\n\t"
		"cli\n\t"
		"popl %0\n\t" : "=m" (current_cpu_state->kernel_eflags) :);
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
	__asm__("pushl %0\n\t"
		"popf" : : "m" (current_cpu_state->kernel_eflags));
}

static inline int
intr_enabled(void)
{
	int ret;

	__asm__("pushf\n\t"
		"popl %0\n\t"
		"andl $0x200, %0\n\t"
		"jz 1f\n\t"
		"movl $1, %0\n\t"
		"1:" : "=g" (ret));
	return ret;
}
#else
void enable_intr(void);
void disable_intr(void);
void restore_eflags(void);
int intr_enabled(void);
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
