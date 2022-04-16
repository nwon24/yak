#include <stdio.h>

void
__libc_fini(void)
{
	__libc_stdio_fini();
}
