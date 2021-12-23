/*
 * proc.c
 * Handles processes.
 * Implements the 'fork' and 'exit' system calls.
 */
#include <stddef.h>

#include <generic/errno.h>
#include <generic/string.h>

#include <kernel/debug.h>
#include <kernel/proc.h>

static struct process process_table[NR_PROC];
static int last_pid = 0;

struct process *current_process;

extern uint32_t _start_user_head, _end_user_head;

int arch_processes_init(uint32_t start, uint32_t size);
int arch_fork(void);

static struct process *get_free_proc(void);

void
processes_init(void)
{
	struct process *proc = FIRST_PROC;

	current_process = proc;
	proc->pid = 0;
	proc->state = PROC_RUNNABLE;
	proc->tty = 0;
	proc->image.vir_code_base = (uint32_t)&_start_user_head;
	proc->image.vir_code_len = &_end_user_head - &_start_user_head;
	if (arch_processes_init((uint32_t)&_start_user_head, &_end_user_head - &_start_user_head) < 0)
		panic("Unable to initialise processes");;
}

/*
 * Main routine for the 'fork' system call.
 */
int
__kernel_fork(void)
{
	struct process *proc;

	if ((proc = get_free_proc()) == NULL)
		return -EAGAIN;
	proc->pid = last_pid;
	proc->state = PROC_RUNNABLE;
	proc->tty = current_process->tty;
	memmove(&proc->image, &current_process->image, sizeof(proc->image));
	return arch_fork();
}

static struct process *
get_free_proc(void)
{
	struct process *proc;

	for (proc = FIRST_PROC; proc < LAST_PROC; proc++) {
		if (!proc->state) {
			last_pid = proc - process_table;
			return proc;
		}
	}
	return NULL;
}
