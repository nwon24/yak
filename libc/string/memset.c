/*
 * memset.c
 * Generic memset().
 */

#include <string.h>

/*
 * TODO: Optimise generic memset().
 */

void *
memset(void *dst, int c, size_t count)
{
	unsigned char *dstp;

	dstp = dst;
	while (count--)
		*dstp++ = (unsigned char)c;
	return dst;
}
