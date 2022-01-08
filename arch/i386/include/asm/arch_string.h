#ifndef ARCH_STRING_H_
#define ARCH_STRING_H_

#include <stddef.h>
#include <stdint.h>

static inline void *
__arch_memcpy(void *dst, const void *src, size_t n)
{
	__asm__("cld; rep; movsb" : : "c" (n), "D" ((uint32_t)dst), "S" ((uint32_t)src) :);
        return dst;
}

static inline void *
__arch_memmove(void *dst, const void *src, size_t n)
{
        if (dst < src)
                return __arch_memcpy(dst, src, n);
        else
		__asm__("std; rep; movsb" : : "c" (n), "D" ((uint32_t)dst + n - 1), "S" ((uint32_t)src + n - 1));
        return dst;
}


static inline int
__arch_memcmp(const void *a, const void *b, size_t n)
{
        int __ret;
        __asm__("cld; repe; cmpsb\n\t"
                "je 1f\n\t"
                "movl $1, %%eax\n\t"
                "jl 1f\n\t"
                "negl %%eax\n"
                "1:" : "=a" (__ret), "+D" (a), "+S" (b), "+c" (n) : "a" (0));
        return __ret;
}

static inline void *
__arch_memset(void *bufp, int c, size_t n)
{
        __asm__("cld; rep; stosb\n\t"
                : "+D" (bufp), "+c" (n) : "a" (c));
        return bufp;
}

#endif /* ARCH_STRING_H_ */
