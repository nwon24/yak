#include <stdarg.h>

#include <unistd.h>
#include <syscall.h>

int
fcntl(int fd, int cmd, ...)
{
	int res;
	va_list args;

	va_start(args, cmd);
	res = syscall(SYS_fcntl, fd, cmd, va_arg(args, int));
	va_end(args);
	return res;
}
