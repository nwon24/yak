#include <unistd.h>
#include <syscall.h>

unsigned int
alarm(unsigned int seconds)
{
	return syscall(SYS_alarm, seconds);
}
