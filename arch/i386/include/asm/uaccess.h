#ifndef UACCESS_H
#define UACCESS_H

#include <stdint.h>

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

#define put_ubyte(ptr, b)	(*(uint8_t *)(ptr) = b)
#define put_uword(ptr, w)	(*(uint16_t *)(ptr) = w)
#define put_ulong(ptr, l)	(*(uint32_t *)(ptr) = l)

#endif /* UACCESS_H */
