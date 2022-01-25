/*
 * signal.c
 * Signal handling part of kernel.
 */

#include <kernel/proc.h>
#include <kernel/debug.h>

#include <generic/errno.h>

static struct signal signal_table[NSIG];

static int send_signal(struct process *target, int sig);
static int sig_permission(struct process *sender, struct process *target);

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
	register_syscall(__NR_signal, (uint32_t)kernel_signal, 2);
	register_syscall(__NR_kill, (uint32_t)kernel_kill, 2);
}

sighandler_t
kernel_signal(int sig, sighandler_t handler)
{
	sighandler_t ret, *ptr;

	if (sig < 1 || sig > NSIG)
		return SIG_ERR;
	if ((sig == SIGKILL || sig == SIGSTOP) && handler == SIG_IGN)
		return SIG_ERR;
	ptr = current_process->sighandlers + sig - 1;
	ret = *ptr;
	*ptr = handler;
	return ret;
}

int
kernel_kill(pid_t pid, int sig)
{
	struct process *proc;
	int eligible = 0;
	int noperm = 0;

	if (sig < 0 || sig > NSIG)
		return -EINVAL;
	if (pid > 0) {
		return send_signal(process_table + pid, sig);
	} else if (pid == 0) {
		for (proc = FIRST_PROC; proc < LAST_PROC; proc++) {
			if (proc->state != PROC_EXITED && proc->pgrp_info.pgid == current_process->pgrp_info.pgid && proc != current_process) {
				if (send_signal(proc, sig) == -EPERM)
					noperm++;
				eligible++;
			}
		}
	} else if (pid == -1) {
		for (proc = FIRST_PROC; proc < LAST_PROC; proc++) {
			if (proc->state != PROC_EXITED && proc->pid != 1 && proc != current_process) {
				if (send_signal(proc, sig) == -EPERM)
					noperm++;
				eligible++;
			}
		}
	} else if (pid < -1) {
		for (proc = FIRST_PROC; proc < LAST_PROC; proc++) {
			if (proc->state != PROC_EXITED
			    && proc->pgrp_info.pgid == current_process->pgrp_info.pgid
			    && proc != current_process
			    && proc->pid == -pid) {
				if (send_signal(proc, sig) == -EPERM)
					noperm++;
				eligible++;
			}
		}
	}
	if (eligible == 0)
		return -ESRCH;
	if (noperm == eligible)
		return -EPERM;
	return 0;
}

static int
send_signal(struct process *target, int sig)
{
	if (sig_permission(current_process, target) != 0)
		return -EPERM;
	if (sig != 0)
		target->sigpending |= (1 << (sig - 1));
	return 0;
}

static int
sig_permission(struct process *sender, struct process *target)
{
	if (sender->uid == target->suid || sender->uid == target->uid)
		return 0;
	if (sender->euid == target->suid || sender->euid == target->uid)
		return 0;
	return 1;
}
