/*
 * strcpy.c
 */

char *
strcpy(char *restrict dest, const char *src)
{
	char *ret = dest;

	while ((*dest++ = *src++));
	return ret;
}
