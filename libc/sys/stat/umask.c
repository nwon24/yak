#include <sys/stat.h>

#include <unistd.h>
#include <syscall.h>

mode_t
umask(mode_t cmask)
{
	return syscall(SYS_umask, cmask);
}
