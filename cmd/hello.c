#include <unistd.h>

int main(void)
{
	write(1, "Hello", 5);
	while (1);
	return 0;
}
