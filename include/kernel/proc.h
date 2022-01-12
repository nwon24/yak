#ifndef PROC_H
#define PROC_H

#include <stdint.h>

#include <asm/cpu_state.h>

#include <fs/fs.h>

#include <kernel/mutex.h>

#define NR_PROC	64

struct proc_image {
	uint32_t vir_code_base;	/* Only readable */
	uint32_t vir_code_len;
	uint32_t vir_data_base; /* Readable and writeable */
	uint32_t vir_data_len;
};

struct process {
	int pid;
	int state;
	int tty;
	int priority;
	int quanta;
	int counter;
	void *sleeping_on;

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

void schedule(void);
void sleep(void *addr);
void wakeup(void *addr);

void adjust_proc_queues(struct process *proc);

enum system_state {
	SYSTEM_SINGLETASKING,
	SYSTEM_MULTITASKING,
	SYSTEM_PANIC,
};

int system_is_multitasking(void);
int system_is_panicing(void);
void system_change_state(enum system_state state);

#define FIRST_PROC	(&process_table[0])
#define LAST_PROC	(&process_table[NR_PROC])

#define PROC_RUNNING	1
#define PROC_BLOCKED	2
#define PROC_RUNNABLE	3

/* In 10s of milliseconds */
#define PROC_QUANTA	10

#define HIGHEST_PRIORITY	(PROC_QUANTA - 1)
#define LOWEST_PRIORITY		0

#endif /* PROC_H */
