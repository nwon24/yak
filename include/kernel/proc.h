#ifndef PROC_H
#define PROC_H

#include <stdint.h>

#include <asm/cpu_state.h>
#include <asm/types.h>

#include <fs/fs.h>

#include <generic/signal.h>

#include <kernel/mutex.h>

#define NR_PROC	64

struct proc_image {
	uint32_t vir_code_base;	/* Only readable */
	uint32_t vir_code_len;
	uint32_t vir_code_count; /* Number of processes sharing text section */
	uint32_t vir_data_base; /* Readable and writeable */
	uint32_t vir_data_len;
};

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
	void *sleeping_on;

	struct signal signals[NSIG];
	struct pgrp_info pgrp_info;

	struct context *context;

	struct proc_image image;
	struct process *queue_next;
	struct process *queue_prev;

	struct generic_filesystem *root_fs;
	void *root_inode;
	struct generic_filesystem *cwd_fs;
	void *cwd_inode;

	struct file *file_table[NR_OPEN];
};

extern struct process *current_process;
extern struct process process_table[];

typedef void (*multitasking_hook)(void);
void add_multitasking_hook(multitasking_hook hook);

void processes_init(void);
int kernel_fork(void);
void kernel_exit(int status);

void schedule(void);
void sleep(void *addr, int type);
void wakeup(void *addr, int ret);

void adjust_proc_queues(struct process *proc);

void signals_init(struct process *proc);

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

#define FIRST_PROC	(&process_table[0])
#define LAST_PROC	(&process_table[NR_PROC])

#define PROC_EXITED	0
#define PROC_RUNNING	1
#define PROC_RUNNABLE	2
#define PROC_SLEEP_INTERRUPTIBLE	3
#define PROC_SLEEP_UNINTERRUPTIBLE	4

#define PROC_SLEEPING(proc)	((proc)->state == PROC_SLEEP_INTERRUPTIBLE || (proc)->state == PROC_SLEEP_UNINTERRUPTIBLE)

/* In 10s of milliseconds */
#define PROC_QUANTA	10

#define HIGHEST_PRIORITY	(PROC_QUANTA - 1)
#define LOWEST_PRIORITY		0

#endif /* PROC_H */
