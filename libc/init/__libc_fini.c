#include <stdio.h>
#include <stdio_impl.h>

void
__libc_fini(void)
{
	__libc_stdio_fini();
}
