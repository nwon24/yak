/*
 * schedule.c
 * Scheduler code. Really quite simple right now.
 * Will also contain code for 'sleep' and 'wakeup'.
 */
#include <stdint.h>

#include <kernel/proc.h>
#include <kernel/debug.h>

void arch_switch_to(int state);

void
schedule(void)
{
	struct process *proc, *old;

	old = current_process;
	for (proc = FIRST_PROC; proc < LAST_PROC; proc++) {
		if (proc == current_process)
			continue;
		if (proc->state == PROC_RUNNABLE) {
			proc->state = PROC_RUNNING;
			current_process->state = PROC_RUNNABLE;
			current_process = proc;
			goto switch_proc;
		}
	}
	/* No processes available to run. Resort to idle */
	current_process = FIRST_PROC;
switch_proc:
	if (current_process != old)
		arch_switch_to(current_process->pid);
}
