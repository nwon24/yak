/*
 * memcmp.c
 * Generic memcmp().
 */

#include <string.h>

/*
 * TODO: Optimise memcmp().
 */

int
memcmp(const void *s1, const void *s2, size_t count)
{
	const unsigned char *s1p, *s2p;

	s1p = s1;
	s2p = s2;
	while (count--) {
		if (*s1p < *s2p)
			return -1;
		if (*s1p > *s2p)
			return 1;
		s1p++;
		s2p++;
	}
	return 0;
}
