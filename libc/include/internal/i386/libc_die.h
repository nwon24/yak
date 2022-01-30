#ifndef _INTERNAL_I386_LIBC_DIE_H
#define _INTERNAL_I386_LIBC_DIE_H

static inline void
__arch_libc_early_die(void)
{
	__asm__("hlt");
}

#define __libc_early_die __arch_libc_early_die

#endif /* _INTERNAL_I386_LIBC_DIE_H */
