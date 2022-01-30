#include <unistd.h>

int main(int argc, char *argv[])
{
	write(1, argv[0], 14);
	write(1, "\r\n", 2);
	if (!fork())
		write(1, "Hello again", 12);
	while (1);
	return 0;
}
