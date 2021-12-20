#ifndef _USER_MODE_H
#define _USER_MODE_H

#include <kernel/config.h>

#ifdef _CONFIG_USE_INLINE_ASM
#include <asm/segment.h>

static inline void
i386_move_to_user(void)
{
	__asm__("movw %%ax, %%ds\n\t"
		"movw %%ax, %%es\n\t"
		"movw %%ax, %%fs\n\t"
		"movw %%ax, %%gs\n\t"
		"movl %%esp, %%edx\n\t"
		"pushl %0\n\t"
		"pushl %%edx\n\t"
		"pushf\n\t"
		"pushl %1\n\t"
		"pushl $1f\n\t"
		"iret\n\t"
		"1:"
		: : "a" (USER_DS_SELECTOR), "i" (USER_CS_SELECTOR) : "%edx");
}
#else
void i386_move_to_user(void);
#endif /* _CONFIG_USE_INLINE_ASM */

#define move_to_user	i386_move_to_user

#endif /* _USER_MODE_H */
