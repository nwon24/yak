#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	int status, fd;

	write(1, argv[0], 14);
	write(1, "\r\n", 2);
	if (!fork()) {
		write(1, "Hello again", 12);
	} else {
		waitpid(-1, &status, 0);
		write(1, "Child has died", 14);
		fd = open("/usr/include/unist.h", O_RDONLY | O_CREAT, S_IRWXU);
		close(fd);
	}
	return 0;
}
