/*
 * proc.c
 * Handles processes.
 */
#include <kernel/debug.h>
#include <kernel/proc.h>

static struct process process_table[NR_PROC];
struct process *current_process;

extern uint32_t _start_user_head, _end_user_head;

int arch_processes_init(uint32_t start, uint32_t size);

void
processes_init(void)
{
	struct process *proc = FIRST_PROC;

	current_process = proc;
	proc->pid = 0;
	proc->state = PROC_RUNNABLE;
	proc->tty = 0;
	if (arch_processes_init((uint32_t)&_start_user_head, &_end_user_head - &_start_user_head) < 0)
		panic("Unable to initialise processes");;
}
