#include <sys/wait.h>
#include <syscall.h>
#include <unistd.h>

pid_t
waitpid(pid_t pid, int *wstatus, int options)
{
	return syscall(SYS_waitpid, pid, wstatus, options);
}
