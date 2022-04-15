/*
 * strncpy.c
 * Safer version of strcpy()
 */
#include <string.h>

char *
strncpy(char *restrict dest, const char *restrict src, size_t len)
{
	char *ret = dest;

	while (len-- && (*dest++ = *src++));
	return ret;
}
