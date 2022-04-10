#include <unistd.h>

int main(void)
{
	write(STDOUT_FILENO, "Hello, world!\n", 14);
	while (1);
	return 0;
}
