#include <unistd.h>

#include <sys/wait.h>
#include <stdio.h>

char *null_argv[] = { NULL, NULL };
char *null_envp[] = { NULL };

int main(void)
{
	int c, i, r;
	char buf[512], *p;

	write(1, "Hello\n", 6);
again:
	write(1, "$ ", 2);
	c = 0;
	r = read(0, buf, 512);
	if (r == 0)
		goto out;
	write(1, buf, r);
	buf[r - 1] = '\0';
	null_argv[0] = buf;
	if (!fork()) {
		execve(buf, (const char **)null_argv, (const char **)null_envp);
		write(1, "?\n", 2);
	} else {
		wait(&i);
		goto again;
	}
out:
	return 0;
}
