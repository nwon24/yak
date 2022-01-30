#include <unistd.h>
#include <syscall.h>

int
execve(const char *path, const char *argv[], const char *envp[])
{
	return syscall(SYS_execve, path, argv, envp);
}
