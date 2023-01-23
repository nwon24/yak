#include <unistd.h>

#include <sys/wait.h>
#include <stdio.h>

const char *hello = "/usr/bin/hello";
const char *hello_argv[] = { NULL };
const char *hello_envp[] = { NULL };

int main(void)
{
	int i;
	char buf[512];

	write(1, "Hello\n", 6);
	while ((i = read(STDIN_FILENO, buf, sizeof(buf))) > 0)
		write(STDOUT_FILENO, buf, i);

	if (!fork()) {
		execve(hello, hello_argv, hello_envp);	
		write(1, "here?\n", 6);
	} else {
		wait(&i);
		write(1, "blah\n", 5);
	}
	write(STDOUT_FILENO, "Done!\n", 6); 
	return 0;
}
