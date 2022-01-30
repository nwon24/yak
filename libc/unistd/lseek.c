#include <unistd.h>
#include <syscall.h>

off_t
lseek(int fd, off_t off, int whence)
{
	return syscall(SYS_lseek, fd, off, whence);
}
