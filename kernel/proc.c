/*
 * proc.c
 * Handles processes.
 * Implements the 'fork' and 'exit' system calls.
 */
#include <kernel/config.h>

#include <stddef.h>

#include <drivers/timer.h>

#include <generic/errno.h>
#include <generic/string.h>

#include <kernel/debug.h>
#include <kernel/proc.h>

#define NR_MULTITASKING_HOOK	CONFIG_DRIVER_NR_DRIVERS

static multitasking_hook multitasking_hooks[NR_MULTITASKING_HOOK];
static int nr_multitasking_hook;

static int last_pid = 0;

struct process process_table[NR_PROC];
struct process *current_process;

enum system_state {
	SYSTEM_SINGLETASKING,
	SYSTEM_MULTITASKING,
};

enum system_state system_state = SYSTEM_SINGLETASKING;

extern uint32_t _start_user_head, _end_user_head;

int arch_processes_init(uint32_t start, uint32_t size);
int arch_fork(int child, struct process *proc);

static struct process *get_free_proc(void);
static void system_change_state(enum system_state state);
static void run_multitasking_hooks(void);

extern struct process *process_queues[];

int
system_is_multitasking(void)
{
	return system_state == SYSTEM_MULTITASKING;
}

/*
 * Add a function to the list of functions to be run
 * just before the systems goes into multitasking mode.
 */
void
add_multitasking_hook(multitasking_hook hook)
{
	multitasking_hooks[nr_multitasking_hook++] = hook;
}

static void
run_multitasking_hooks(void)
{
	int i;

	for (i = 0; i < nr_multitasking_hook; ++i)
		multitasking_hooks[i]();
}

static void
system_change_state(enum system_state state)
{
	system_state = state;
}

void
processes_init(void)
{
	struct process *proc = FIRST_PROC;
	int i;

	for (i = 0; i < PROC_QUANTA; i++)
		process_queues[i] = NULL;
	current_process = proc;
	proc->pid = 0;
	proc->priority = 0;
	proc->state = PROC_RUNNABLE;
	proc->tty = 0;
	proc->quanta = proc->priority;
	proc->counter = proc->quanta;
	proc->image.vir_code_base = (uint32_t)&_start_user_head;
	proc->image.vir_code_len = &_end_user_head - &_start_user_head;
	process_queues[proc->priority] = proc;
	proc->queue_next = proc->queue_prev = NULL;
	system_change_state(SYSTEM_MULTITASKING);
	run_multitasking_hooks();
	/* Timer init should be called from 'arch_processes_init' */
	if (arch_processes_init((uint32_t)&_start_user_head, &_end_user_head - &_start_user_head) < 0)
		panic("Unable to initialise processes");;
}

/*
 * Main routine for the 'fork' system call.
 */
int
kernel_fork(void)
{
	struct process *proc;

	if ((proc = get_free_proc()) == NULL)
		return -EAGAIN;
	if (arch_fork(last_pid, proc) < 0)
		return -EAGAIN;
	if (!current_process->priority) {
		/* First fork */
		proc->priority = PROC_QUANTA - 1;
		proc->quanta = proc->priority;
	} else {
		proc->priority = current_process->priority;
		proc->quanta = current_process->priority;
	}
	proc->pid = last_pid;
	proc->state = PROC_RUNNABLE;
	proc->tty = current_process->tty;
	proc->counter = current_process->quanta;
	proc->queue_next = proc->queue_prev = NULL;
	memmove(&proc->image, &current_process->image, sizeof(proc->image));
	adjust_proc_queues(proc);
	return last_pid;
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
