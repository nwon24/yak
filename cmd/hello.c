#include <unistd.h>

int main(void)
{
	write(STDOUT_FILENO, "Hello, world!\n", 14);
	return 0;
}
