/*
 * open.c
 * The 'open' system call.
 */
#include <asm/paging.h>
#include <asm/types.h>

#include <fs/fs.h>

#include <generic/errno.h>

#include <kernel/debug.h>
#include <kernel/proc.h>

struct file file_table[NR_FILE];

static inline struct file *
get_free_file(void)
{
	struct file *fp;

	for (fp = file_table; fp < file_table + NR_FILE; fp++) {
		if (fp->f_count == 0)
			return fp;
	}
	return NULL;
}

static inline int
find_fd(void)
{
	int i;

	for (i = 0; i < NR_FILE; i++) {
		if (current_process->file_table[i] == NULL)
			return i;
	}
	return -1;
}

int
kernel_open(const char *path, int flags, mode_t mode)
{
	struct file *fp;
	int fd;

	if (path == NULL || !check_user_ptr((void *)path))
		return -EINVAL;
	if ((fd = find_fd()) < 0)
		return -ENFILE;
	if ((fp = get_free_file()) == NULL)
		return -ENFILE;
	printk("here\r\n");
	current_process->file_table[fd] = fp;
	fp->f_pos = 0;
	fp->f_mode = mode;
	fp->f_flags = flags;
	fp->f_count = 1;
	return fd;
}