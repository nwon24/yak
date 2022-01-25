/*
 * signal.c
 * Signal handling part of kernel.
 */

#include <kernel/proc.h>

#include <kernel/debug.h>

static struct signal signal_table[NSIG];

static inline void
signal_init(int signal, enum sig_def_action action)
{
	if (signal == 0)
		return;
	signal_table[signal - 1].def_action = action;
	signal_table[signal - 1].sig = signal;
}

void
signals_init(void)
{
	signal_init(SIGBUS, SIGACTION_A);
	signal_init(SIGCHLD, SIGACTION_I);
	signal_init(SIGCONT, SIGACTION_C);
	signal_init(SIGPOLL, SIGACTION_T);
	signal_init(SIGPROF, SIGACTION_T);
	signal_init(SIGSTOP, SIGACTION_S);
	signal_init(SIGSYS, SIGACTION_A);
	signal_init(SIGTSTP, SIGACTION_S);
	signal_init(SIGTTIN, SIGACTION_S);
	signal_init(SIGTTOU, SIGACTION_S);
	signal_init(SIGUSR1, SIGACTION_T);
	signal_init(SIGUSR2, SIGACTION_T);
	signal_init(SIGURG, SIGACTION_I);
	signal_init(SIGVTALRM, SIGACTION_T);
	signal_init(SIGXCPU, SIGACTION_A);
	signal_init(SIGXFSZ, SIGACTION_A);
	signal_init(SIGWINCH, SIGACTION_I);
	signal_init(SIGHUP, SIGACTION_T);
	signal_init(SIGINT, SIGACTION_T);
	signal_init(SIGQUIT, SIGACTION_A);
	signal_init(SIGILL, SIGACTION_A);
	signal_init(SIGTRAP, SIGACTION_A);
	signal_init(SIGABRT, SIGACTION_A);
	signal_init(SIGFPE, SIGACTION_A);
	signal_init(SIGKILL, SIGACTION_T);
	signal_init(SIGSEGV, SIGACTION_A);
	signal_init(SIGPIPE, SIGACTION_T);
	signal_init(SIGALRM, SIGACTION_T);
	signal_init(SIGTERM, SIGACTION_T);
}
