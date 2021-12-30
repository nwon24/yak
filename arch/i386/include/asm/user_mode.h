#ifndef USER_MODE_H
#define USER_MODE_H

#include <kernel/config.h>

#ifdef CONFIG_USE_INLINE_ASM
#include <asm/segment.h>

static inline void
i386_move_to_user(uint32_t entry, uint32_t esp)
{
	__asm__("movw %%ax, %%ds\n\t"
		"movw %%ax, %%es\n\t"
		"movw %%ax, %%fs\n\t"
		"movw %%ax, %%gs\n\t"
		"pushl %0\n\t"
		"pushl %1\n\t"
		"pushf\n\t"
		"pushl %2\n\t"
		"pushl %3\n\t"
		"iret\n\t"
		: : "a" (USER_DS_SELECTOR), "i" (esp), "i" (USER_CS_SELECTOR), "i" (entry));
}
#else
void i386_move_to_user(uint32_t entry, uint32_t esp);
#endif /* CONFIG_USE_INLINE_ASM */

#define move_to_user	i386_move_to_user

#endif /* USER_MODE_H */
