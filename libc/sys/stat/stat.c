#include <sys/stat.h>

#include <unistd.h>
#include <syscall.h>

int
stat(const char *restrict path, struct stat *restrict statbuf)
{
	return syscall(SYS_stat, path, statbuf);
}
