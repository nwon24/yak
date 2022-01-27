/*
 * fs/fcntl.c
 */
#include <fs/fs.h>

#include <generic/fcntl.h>
#include <generic/errno.h>

#include <kernel/proc.h>

static int _dup(int fd, int new, int cloexec);

static int
_dup(int fd, int new, int cloexec)
{
	struct file **fnew, *old;
	int ret;

	if (fd < 0 || fd >= NR_OPEN)
		return -EBADF;
	old = current_process->file_table[fd];
	if (old == NULL)
		return -EBADF;
	for (fnew = current_process->file_table + new; fnew < current_process->file_table + NR_OPEN; fnew++) {
		if (*fnew == NULL)
			break;
	}
	if (fnew >= current_process->file_table + NR_OPEN)
		return -EMFILE;
	*fnew = old;
	(*fnew)->f_count++;
	ret = fnew - current_process->file_table;
	if (cloexec)
		current_process->close_on_exec &= ~(1 << ret);
	return ret;
}

int
kernel_dup(int fd)
{
	return _dup(fd, 0, 0);
}

int
kernel_dup2(int fd1, int fd2)
{
	struct file *fp;

	if (fd2 < 0 || fd2 >= NR_OPEN)
		return -EBADF;
	fp = current_process->file_table[fd1];
	if (fp != NULL && fd1 == fd2)
		return fd1;
	if (fp == NULL)
		return -EBADF;
	kernel_close(fd2);
	return _dup(fd1, fd2, 0);
}

int
kernel_fcntl(int fd, int cmd, int arg)
{
	struct file *fp;

	if (fd < 0 || fd >= NR_OPEN)
		return -EBADF;
	fp = current_process->file_table[fd];
	if (fp == NULL)
		return -EBADF;
	switch (cmd) {
	case F_DUPFD:
		return _dup(fd, arg, 0);
	case F_DUPFD_CLOEXEC:
		return _dup(fd, arg, 1);
	case F_GETFD:
		return (current_process->close_on_exec >> fd) & 1;
	case F_SETFD:
		if (arg & FD_CLOEXEC)
			current_process->close_on_exec |= (1 << fd);
		else
			current_process->close_on_exec &= ~(1 << fd);
		return 0;
	case F_GETFL:
		return fp->f_flags;
	case F_SETFL:
		fp->f_flags = arg;
		return 0;
	case F_GETOWN:
	case F_SETOWN:
	case F_GETLK:
	case F_SETLK:
	case F_SETLKW:
		return -ENOSYS;
	default:
		return -EINVAL;
	}
}
