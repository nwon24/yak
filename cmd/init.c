#include <unistd.h>

int main(int argc, char *argv[])
{
	int i;
	char buf[512];

	write(STDOUT_FILENO, "Hello, world!\n", 14);
	while ((i = read(STDIN_FILENO, buf, sizeof(buf))) > 0)
		write(STDOUT_FILENO, buf, i);
	write(STDOUT_FILENO, "Done!\n", 6);
	return 0;
}
