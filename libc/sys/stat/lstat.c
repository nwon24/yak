#include <sys/stat.h>

#include <unistd.h>
#include <syscall.h>

int
lstat(const char *restrict path, struct stat *restrict statbuf)
{
	return syscall(SYS_lstat, path, statbuf);
}
