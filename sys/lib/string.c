/*
 * string.c
 * Generic versions of memory functions for freestanding C environment.
 */

#include <stddef.h>

void *
memcpy(void *dst, const void *src, size_t count)
{
	unsigned char *dstp = (unsigned char *)dst;
	const unsigned char *srcp = (const unsigned char *)src;

	while (count--)
		*dstp++ = *srcp++;
	return dst;
}

void *
memmove(void *dst, const void *src, size_t count)
{
	unsigned char *dstp = (unsigned char *)dst;
	const unsigned char *srcp = (const unsigned char *)src;

	if (dstp < srcp) {
		while (count--)
			*dstp++ = *srcp++;
	} else {
		while (count--)
			*(dstp + count) = *(srcp + count);
	}
	return dst;
}

int
memcmp(const void *str1, const void *str2, size_t count)
{
	const unsigned char *p1 = (const unsigned char *)str1;
	const unsigned char *p2 = (const unsigned char *)str2;

	while (count--) {
		if (*p1 < *p2)
			return -1;
		else if (*p2 < *p1)
			return 1;
		p1++;
		p2++;
	}
	return 0;
}

void *
memset(void *bufp, int c, size_t count)
{
	unsigned char *p = (unsigned char *)bufp;

	while (count--)
		*p++ = (unsigned char)c;
	return bufp;
}

size_t
strlen(const char *str)
{
	const char *p = str;

	while (*p)
		p++;
	return p - str;
}
