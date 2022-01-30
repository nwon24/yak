#include <stdarg.h>

#include <unistd.h>
#include <syscall.h>

int
open(const char *path, int flags, ...)
{
	va_list args;
	int res;

	va_start(args, flags);
	res = syscall(SYS_open, path, flags, va_arg(args, int));
	va_end(args);
	return res;
}
