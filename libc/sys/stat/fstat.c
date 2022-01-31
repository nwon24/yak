#include <sys/stat.h>

#include <unistd.h>
#include <syscall.h>

int
fstat(int fd, struct stat *statbuf)
{
	return syscall(SYS_fstat, fd, statbuf);
}
