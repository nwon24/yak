#ifndef GENERIC_STRING_H
#define GENERIC_STRING_H

#ifndef __ASSEMBLER__

#include <stddef.h>

#include <kernel/config.h>

#ifdef CONFIG_USE_ARCH_STRING_H
#include <asm/arch_string.h>
#define memcpy	__arch_memcpy
#define memmove __arch_memmove
#define memcmp __arch_memcmp
#define memset __arch_memset
#else
void *memcpy(void *dst, const void *src, size_t count);
void *memmove(void *dst, const void *src, size_t count);
int memcmp(const void *str1, const void *str2, size_t count);
void *memset(void *bufp, int c, size_t count);
#endif /* CONFIG_ARCH_USE_STRING_H */

size_t strlen(const char *str);

#endif /* __ASSEMBLER__ */

#endif /* GENERIC_STRING_H */
