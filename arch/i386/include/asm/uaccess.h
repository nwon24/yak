#ifndef UACCESS_H
#define UACCESS_H

#include <stdint.h>
#include <stddef.h>
/*
 * With the way paging is set up for x86,
 * user space data can be accessed without any special
 * function. However, hide it in a macro in case this
 * ever changes. We always just need to check the pointer
 * is okay and mapped.
 */
#define get_ubyte(ptr)	(*(uint8_t *)(ptr))
#define get_uword(ptr)	(*(uint16_t *)(ptr))
#define get_ulong(ptr)	(*(uint32_t *)(ptr))

#define put_ubyte(ptr, b)	(*(uint8_t *)(ptr) = (b))
#define put_uword(ptr, w)	(*(uint16_t *)(ptr) = (w))
#define put_ulong(ptr, l)	(*(uint32_t *)(ptr) = (l))

/*
 * Don't need to worry about overlapping memory - kernel memory
 * and user memory shouldn't be overlapping.
 */
static inline void *
cp_to_user(void *dest, void *src, size_t count)
{
	unsigned char *dstp, *srcp;

	dstp = (unsigned char *)dest;
	srcp = (unsigned char *)src;

	while (count--) {
		put_ubyte(dstp, *srcp);
		dstp++;
		srcp++;
	}
	return dest;
}

static inline void *
cp_from_user(void *dest, void *src, size_t count)
{
	unsigned char *dstp, *srcp;

	dstp = (unsigned char *)dest;
	scrp = (unsigned char *)src;

	while (count--) {
		*dstp = get_ubyte(src);
		dstp++;
		srcp++;
	}
}

#endif /* UACCESS_H */
