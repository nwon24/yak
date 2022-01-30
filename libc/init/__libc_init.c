#include <unistd.h>
#include <fcntl.h>
#include <libc_die.h>

void
__libc_init(int argc, const char *argv[])
{
	close(0);
	close(1);
	close(2);
	if (open("/dev/tty", O_RDWR) < 0)
		__libc_early_die();
	dup(0);
	dup(0);
}
