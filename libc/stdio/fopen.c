/*
 * fopen.c
 */
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

void __set_errno(int err);

FILE *
fopen(const char *restrict path, const char *restrict mode)
{
	int flags, append, trunc, open_flags;
	mode_t creat_mode;
	FILE *fp;

	flags = 0;
	append = trunc = 0;
	creat_mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
	open_flags = 0;
	if (*mode == 'r') {
		flags |= __READ;
	} else if (*mode == 'w') {
		flags |= __WRITE;
		open_flags |= O_CREAT;
	} else if (*mode == 'a') {
		flags |= __WRITE;
		open_flags |= O_CREAT;
		append = 1;
	} else {
		__set_errno(EINVAL);
		return NULL;
	}
	if (*++mode == 'b')
		mode++;
	if (*mode == '+') {
		if (flags & __READ) {
			flags |= __WRITE;
		} else if (flags & __WRITE) {
			flags |= __READ;
			if (append == 0)
				trunc = 1;
		}
	} else if (*mode != 'b' && *mode != '\0') {
		__set_errno(EINVAL);
		return NULL;
	}
	for (fp = __file_table; fp < __file_table + FOPEN_MAX; fp++) {
		if (fp->flags == 0)
			break;
	}
	if (fp >= __file_table + FOPEN_MAX) {
		__set_errno(EMFILE);
		return NULL;
	}
	fp->flags = flags;
	if ((flags & __WRITE) && (flags & __READ))
		open_flags = O_RDWR;
	else if (!(flags & __WRITE) && (flags & __READ))
		 open_flags = O_RDONLY;
	else if ((flags & __WRITE) && !(flags & __READ))
		open_flags = O_WRONLY;
	if (append)
		fp->fd = open(path, open_flags | O_APPEND, creat_mode);
	else if (trunc)
		fp->fd = open(path, open_flags | O_TRUNC, creat_mode);
	else
		fp->fd = open(path, open_flags, creat_mode);
	if (fp->fd < 0)
		return NULL;
	fp->wptr = fp->wbuf;
	fp->rptr = fp->rbuf;
	fp->wcount = fp->rcount = 0;
	return fp;
}
