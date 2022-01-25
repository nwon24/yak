/*
 * signal.c
 * Signal handling part of kernel.
 */

#include <kernel/proc.h>

#include <kernel/debug.h>

static inline void
signal_init(struct process *proc, int signal, enum sig_def_action action)
{
	if (signal == 0)
		return;
	proc->signals[signal - 1].def_action = action;
	proc->signals[signal - 1].sig = signal;
}

void
signals_init(struct process *proc)
{
	signal_init(proc, SIGBUS, SIGACTION_A);
	signal_init(proc, SIGCHLD, SIGACTION_I);
	signal_init(proc, SIGCONT, SIGACTION_C);
	signal_init(proc, SIGPOLL, SIGACTION_T);
	signal_init(proc, SIGPROF, SIGACTION_T);
	signal_init(proc, SIGSTOP, SIGACTION_S);
	signal_init(proc, SIGSYS, SIGACTION_A);
	signal_init(proc, SIGTSTP, SIGACTION_S);
	signal_init(proc, SIGTTIN, SIGACTION_S);
	signal_init(proc, SIGTTOU, SIGACTION_S);
	signal_init(proc, SIGUSR1, SIGACTION_T);
	signal_init(proc, SIGUSR2, SIGACTION_T);
	signal_init(proc, SIGURG, SIGACTION_I);
	signal_init(proc, SIGVTALRM, SIGACTION_T);
	signal_init(proc, SIGXCPU, SIGACTION_A);
	signal_init(proc, SIGXFSZ, SIGACTION_A);
	signal_init(proc, SIGWINCH, SIGACTION_I);
	signal_init(proc, SIGHUP, SIGACTION_T);
	signal_init(proc, SIGINT, SIGACTION_T);
	signal_init(proc, SIGQUIT, SIGACTION_A);
	signal_init(proc, SIGILL, SIGACTION_A);
	signal_init(proc, SIGTRAP, SIGACTION_A);
	signal_init(proc, SIGABRT, SIGACTION_A);
	signal_init(proc, SIGFPE, SIGACTION_A);
	signal_init(proc, SIGKILL, SIGACTION_T);
	signal_init(proc, SIGSEGV, SIGACTION_A);
	signal_init(proc, SIGPIPE, SIGACTION_T);
	signal_init(proc, SIGALRM, SIGACTION_T);
	signal_init(proc, SIGTERM, SIGACTION_T);
}
