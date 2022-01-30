#include <unistd.h>
#include <stddef.h>
#include <syscall.h>

ssize_t
write(int fd, void *buf, size_t count)
{
	return syscall(SYS_write, fd, buf, count);
}
