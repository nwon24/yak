/*
 * schedule.c
 * Scheduler code. Really quite simple right now.
 * Will also contain code for 'sleep' and 'wakeup'.
 */
#include <stdint.h>
#include <stddef.h>

#include <asm/interrupts.h>

#include <drivers/timer.h>

#include <kernel/proc.h>
#include <kernel/mutex.h>

#include <kernel/debug.h>

#define IN_A_QUEUE(proc)	((proc)->queue_next != NULL && (proc)->queue_prev != NULL)

void arch_switch_to(struct process *prev, struct process *new);

struct process *process_queues[PROC_QUANTA];

static void add_to_queue(struct process *proc);
static void remove_from_queue(struct process *proc);
static int nice_to_pri(int nice);

void
schedule(void)
{
	struct process **proc, *old, *tmp;

	for (tmp = FIRST_PROC; tmp < LAST_PROC; tmp++) {
		if (tmp->alarm != NO_ALARM && (unsigned int)tmp->alarm < timer_ticks) {
			tmp->sigpending |= (1 << (SIGALRM - 1));
			tmp->alarm = NO_ALARM;
		}
		if (tmp->tty_block != NO_TTY_BLOCK && tmp->tty_block < timer_ticks) {
			tmp->state = PROC_RUNNABLE;
			adjust_proc_queues(tmp);
			tmp->tty_block = NO_TTY_BLOCK;
		}
		if (tmp->state == PROC_SLEEP_INTERRUPTIBLE && tmp->sigpending) {
			tmp->state = PROC_RUNNABLE;
			adjust_proc_queues(tmp);
		}
	}
	old = current_process;
	for (proc = process_queues + HIGHEST_PRIORITY; proc >= process_queues + LOWEST_PRIORITY; proc--) {
		if (*proc == NULL)
			continue;
		(*proc)->state = PROC_RUNNING;
		current_process = *proc;
		goto switch_proc;
	}
	/* No processes available to run. Resort to idle */
	current_process = process_queues[LOWEST_PRIORITY];
switch_proc:
	if (current_process != old)
		arch_switch_to(old, current_process);
}


void
sleep(void *addr, int type)
{
	disable_intr();
	if (current_process == FIRST_PROC)
		panic("Trying to sleep on first process");
	if (type != PROC_SLEEP_INTERRUPTIBLE && type != PROC_SLEEP_UNINTERRUPTIBLE)
		panic("sleep: bad type");
	current_process->state = type;
	current_process->sleeping_on = addr;
	adjust_proc_queues(current_process);
	schedule();
	restore_intr_state();
}

void
wakeup(void *addr, int ret)
{
	struct process *proc;
	int sched = 0;

	disable_intr();
	for (proc = FIRST_PROC; proc < LAST_PROC; proc++) {
		if (PROC_SLEEPING(proc) && proc->sleeping_on == addr) {
			sched = 1;
			proc->state = PROC_RUNNABLE;
			proc->sleeping_on = NULL;
			adjust_proc_queues(proc);
		}
	}
	if (sched && ret == WAKEUP_SWITCH)
		schedule();
	restore_intr_state();
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
			process_queues[proc->priority] = proc->queue_next;
		}
	}
	/* Make it clear that it is not in a queue. */
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
	tmp->queue_prev = proc;
}

int
kernel_nice(int inc)
{
	int *nice = &current_process->nice;

	*nice += inc;
	if (*nice > 19)
		*nice = 19;
	else if (*nice < -20)
		*nice = -20;
	current_process->priority = nice_to_pri(*nice);
	current_process->counter = current_process->quanta = current_process->priority;
	adjust_proc_queues(current_process);
	return *nice;
}

static int
nice_to_pri(int nice)
{
	int tmp, rat;

	rat = 40 / PROC_QUANTA;
	tmp = -(nice / rat);
	return tmp + rat;
}
