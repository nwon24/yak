/*
 * memmove.c
 * Generic memmove().
 */

#include <string.h>

/*
 * TODO: Optimise generic memmove().
 */
void *
memmove(void *dst, const void *src, size_t count)
{
	unsigned char *dstp;
	const unsigned char *srcp;

	dstp = dst;
	srcp = src;
	if (dstp < srcp) {
		while (count--)
			*dstp++ = *srcp++;
	} else {
		while (count--)
			*(dstp + count) = *(srcp + count);
	}
	return dst;
}
