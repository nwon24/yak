/*
 * memcpy.c
 * Generic memcpy().
 */

#include <string.h>

/*
 * TODO: Optimise generic memcpy()
 */

void *
memcpy(void *restrict dst, const void *restrict src, size_t count)
{
	unsigned char *dstp;
	const unsigned char *srcp;

	dstp = dst;
	srcp = src;
	while (count--)
		*dstp++ = *srcp++;
	return dst;
}
