#include <unistd.h>

#include <sys/wait.h>
#include <stdio.h>

int main(void)
{
	int i;
	char buf[512];

/*	write(1, "Hello\n", 6); */
	while ((i = read(STDIN_FILENO, buf, sizeof(buf))) > 0)
		write(STDOUT_FILENO, buf, i);
	write(STDOUT_FILENO, "Done!\n", 6); 
	return 0;
}
