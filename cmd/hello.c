#include <unistd.h>

int main(int argc, char *argv[])
{
	write(1, argv[0], 14);
	while (1);
	return 0;
}
