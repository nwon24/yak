#include <unistd.h>
#include <syscall.h>

int
symlink(const char *path1, const char *path2)
{
	return syscall(SYS_symlink, path1, path2);
}
