/*
 * exit.c
 * Library exit routine - not system call.
 */
void _exit(int status);
void __libc_fini(void);

__attribute__((noreturn)) void
exit(int status)
{
	/* TODO: Implement calling of functions registered using atexit() */
	__libc_fini();
	_exit(status);
	__builtin_unreachable();
}
