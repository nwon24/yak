#ifndef _GENERIC_STRING_H
#define _GENERIC_STRING_H

#ifndef _ASSEMBLY_

#include <stddef.h>

void *memcpy(void *dst, const void *src, size_t count);
void *memmove(void *dst, const void *src, size_t count);
int memcmp(const void *str1, const void *str2, size_t count);
void *memset(void *bufp, int c, size_t count);

size_t strlen(const char *str);

#endif /* _ASSEMBLY_ */

#endif /* _GENERIC_STRING_H */
