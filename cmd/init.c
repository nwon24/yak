#include <unistd.h>

#include <sys/wait.h>

int main(int argc, char *argv[])
{
	int i;
	char buf[512];
	const char *argv_new[] = { "/usr/bin/hello", NULL };
	const char *envp_new[] = { "HOME=/usr/root", NULL };

	if (!fork()) {
		execve("/usr/bin/hello", argv_new, envp_new);
	}
	while ((i = read(STDIN_FILENO, buf, sizeof(buf))) > 0)
		write(STDOUT_FILENO, buf, i);
	write(STDOUT_FILENO, "Done!\n", 6);
	return 0;
}
