/*
 * schedule.c
 * Scheduler code. Really quite simple right now.
 * Will also contain code for 'sleep' and 'wakeup'.
 */
#include <stdint.h>
#include <stddef.h>

#include <asm/interrupts.h>

#include <kernel/proc.h>
#include <kernel/mutex.h>

#include <kernel/debug.h>

#define IN_A_QUEUE(proc)	((proc)->queue_next != NULL && (proc)->queue_prev != NULL)

void arch_switch_to(struct process *prev, struct process *new);

struct process *process_queues[PROC_QUANTA];

static void add_to_queue(struct process *proc);
static void remove_from_queue(struct process *proc);

void
schedule(void)
{
	struct process *proc, *old;

	old = current_process;
	for (proc = process_queues[HIGHEST_PRIORITY]; proc >= process_queues[LOWEST_PRIORITY]; proc--) {
		if (!proc)
			continue;
		if (proc->state == PROC_BLOCKED)
			__asm__("cli; hlt");
		proc->state = PROC_RUNNING;
		if (current_process != FIRST_PROC && current_process->quanta != current_process->counter)
			current_process->priority = current_process->quanta / (current_process->quanta - current_process->counter);
		current_process->quanta = current_process->priority;
		current_process->counter = current_process->quanta;
		adjust_proc_queues(current_process);
		current_process = proc;
		goto switch_proc;
	}
	/* No processes available to run. Resort to idle */
	current_process = process_queues[LOWEST_PRIORITY];
switch_proc:
	if (current_process != old)
		arch_switch_to(old, current_process);
}


void
sleep(void *addr)
{
	if (current_process == FIRST_PROC)
		panic("Trying to sleep on first process");
	disable_intr();
	current_process->state = PROC_BLOCKED;
	current_process->sleeping_on = addr;
	adjust_proc_queues(current_process);
	schedule();
	enable_intr();
}

void
wakeup(void *addr)
{
	struct process *proc;
	int sched = 0;

	disable_intr();
	for (proc = FIRST_PROC; proc < LAST_PROC; proc++) {
		if (proc->state == PROC_BLOCKED && proc->sleeping_on == addr) {
			sched = 1;
			proc->state = PROC_RUNNABLE;
			proc->sleeping_on = NULL;
			adjust_proc_queues(proc);
		}
	}
	if (sched)
		schedule();
	enable_intr();
}

void
adjust_proc_queues(struct process *proc)
{
	if (IN_A_QUEUE(proc))
		remove_from_queue(proc);
	if (proc->state == PROC_RUNNABLE || proc->state == PROC_RUNNING)
		add_to_queue(proc);
}

/*
 * The process queues are doubly linked lists. If a process is on a queue,
 * both of its pointers should not be NULL.
 */
static void
remove_from_queue(struct process *proc)
{
	if (proc->queue_prev == NULL && proc->queue_next == NULL)
		/* Not in a queue, should not happen */
		return;
	if (proc->queue_prev == NULL || proc->queue_next == NULL) {
		panic("Process queues corrupted");
	} else {
		if (proc->queue_next == proc && proc->queue_prev == proc) {
			/* Only process in queue */
			process_queues[proc->priority] = NULL;
	        } else {
			proc->queue_next->queue_prev = proc->queue_prev;
			proc->queue_prev->queue_next = proc->queue_next;
		}
	}
	proc->queue_next = proc->queue_prev = NULL;
}

static void
add_to_queue(struct process *proc)
{
	struct process *p, *tmp;

	if ((p = process_queues[proc->priority]) == NULL) {
		process_queues[proc->priority] = proc;
		proc->queue_prev = proc->queue_next = proc;
		return;
	}
	tmp = p;
	for ( ; p->queue_next != tmp; p = p->queue_next);
	p->queue_next = proc;
	proc->queue_prev = p;
	proc->queue_next = tmp;
}
