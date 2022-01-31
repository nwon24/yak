#include <unistd.h>
#include <syscall.h>

int
nice(int inc)
{
	return syscall(SYS_nice, inc);
}
