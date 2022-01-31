#ifndef PROC_H
#define PROC_H

#define SIG_RESTORER	212

#ifndef __ASSEMBLER__

#include <stdint.h>

#include <asm/cpu_state.h>
#include <asm/types.h>

#include <fs/fs.h>

#include <generic/signal.h>

#include <kernel/mutex.h>
#include <kernel/exec.h>

#define NR_PROC	64

#define NO_ALARM	0

enum sig_def_action {
	SIGACTION_T,	/* Abnormal termination */
	SIGACTION_A,	/* Abnormal termination with addition actions (e.g., core dump) */
	SIGACTION_I,	/* Ignore signal */
	SIGACTION_S,	/* Stop process */
	SIGACTION_C,	/* Continue the process if stopped, otherwise, ignore */
};

struct signal {
	int sig;
	int def_action;
	void (*func)(int);
};

/*
 * Process group information.
 */
struct pgrp_info {
	int leader;
	int sid;
	int pgid;
};

struct process {
	int pid;
	int ppid;
	uid_t uid;
	uid_t euid;
	uid_t suid;
	uid_t gid;
	uid_t egid;
	uid_t sgid;
	int umask;
	int state;
	int tty;
	int priority;
	int quanta;
	int counter;
	uint8_t exit_code;
	void *sleeping_on;

	struct pgrp_info pgrp_info;

	sighandler_t sighandlers[NSIG];	/* Function pointers to signal handlers */
	int sigmask;			/* Signal masks */
	int sigpending;			/* Pending signals */
	void *sigrestorer;		/* Restorer code in user space */

	unsigned int alarm;

	int nice;

	struct context *context;

	struct exec_image image;
	struct process *queue_next;
	struct process *queue_prev;

	struct generic_filesystem *root_fs;
	struct generic_inode root_inode;
	struct generic_filesystem *cwd_fs;
	struct generic_inode cwd_inode;

	struct file *file_table[NR_OPEN];
	int close_on_exec;
};

extern struct process *current_process;
extern struct process process_table[];

typedef void (*multitasking_hook)(void);
void add_multitasking_hook(multitasking_hook hook);

void processes_init(void);
int kernel_fork(void);
void kernel_exit(int status);
int kernel_execve(const char *path, const char *argv[], const char *envp[]);
sighandler_t kernel_signal(int sig, sighandler_t handler);
int kernel_kill(pid_t pid, int sig);
int kernel_alarm(unsigned int seconds);
int kernel_nice(int inc);

void schedule(void);
void sleep(void *addr, int type);
void wakeup(void *addr, int ret);

void adjust_proc_queues(struct process *proc);

void signals_init(void);
int kernel_pause(void);

pid_t kernel_waitpid(pid_t pid, int *stat_loc, int options);

enum system_state {
	SYSTEM_SINGLETASKING,
	SYSTEM_MULTITASKING,
	SYSTEM_PANIC,
};

int system_is_multitasking(void);
int system_is_panicing(void);
void system_change_state(enum system_state state);

enum {
	WAKEUP_RETURN,
	WAKEUP_SWITCH
};

#endif /* __ASSEMBLER__ */

#define FIRST_PROC	(&process_table[0])
#define LAST_PROC	(&process_table[NR_PROC])

#define PROC_EXITED	0
#define PROC_ZOMBIE	1
#define PROC_RUNNING	2
#define PROC_RUNNABLE	3
#define PROC_SLEEP_INTERRUPTIBLE	4
#define PROC_SLEEP_UNINTERRUPTIBLE	5
#define PROC_STOPPED	6

#define PROC_SLEEPING(proc)	((proc)->state == PROC_SLEEP_INTERRUPTIBLE || (proc)->state == PROC_SLEEP_UNINTERRUPTIBLE)

/* In 10s of milliseconds */
#define PROC_QUANTA	10

#define HIGHEST_PRIORITY	(PROC_QUANTA - 1)
#define LOWEST_PRIORITY		0

#endif /* PROC_H */
