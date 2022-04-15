/*
 * stdio.c
 * Internal misc. stuff that is not visible to programs (__*)
 */
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include <stdio_impl.h>

FILE __file_table[FOPEN_MAX] = {
	{ .flags = __READ, .fd = STDIN_FILENO, .rcount = 0, .wcount = 0 },
	{ .flags = __WRITE, .fd = STDOUT_FILENO, .rcount = 0, .wcount = 0 },
	{ .flags = __WRITE, .fd = STDERR_FILENO, .rcount = 0, .wcount = 0 },
};

FILE *stdin;
FILE *stdout;
FILE *stderr;

void
__libc_stdio_init(void)
{
	FILE *fp;

	stdin = __file_table + 0;
	stdout = __file_table + 1;
	stderr = __file_table + 2;
	for (fp = __file_table; fp < __file_table + 3; fp++) {
		fp->rptr = fp->rbuf;
		fp->wptr = fp->wbuf;
	}
}

void
__libc_stdio_fini(void)
{
	FILE *fp;

	for (fp = __file_table; fp < __file_table + FOPEN_MAX; fp++) {
		if (fp->flags && fp->wptr != fp->wbuf)
			write(fp->fd, fp->wbuf, fp->wcount);
	}
}

int
__fillbuf(FILE *fp)
{
	ssize_t i;

	if (!(fp->flags & __READ)) {
		__set_errno(EBADF);
		fp->flags |= __ERR;
		return EOF;
	}
	if ((i = read(fp->fd, fp->rbuf, __FILE_BUFSIZ)) <= 0) {
		fp->flags |= (i < 0) ? __ERR : __EOF;
		return EOF;
	}
	fp->flags &= ~__EOF;
	fp->rcount = i;
	fp->rptr = fp->rbuf;
	fp->rcount--;
	return (unsigned char)*fp->rptr++;
}

void
__flushbuf(FILE *fp, int c)
{
	ssize_t i;

	if (!(fp->flags & __WRITE)) {
		__set_errno(EBADF);
		fp->flags |= __ERR;
		return;
	}
	if ((i = write(fp->fd, fp->wbuf, fp->wcount)) < 0) {
		fp->flags |= __ERR;
		return;
	}
	fp->wptr = fp->wbuf;
	fp->wcount = 1;
	*fp->wptr++ = (unsigned char)c;
}
