/*
 * schedule.c
 * Scheduler code. Really quite simple right now.
 * Will also contain code for 'sleep' and 'wakeup'.
 */
#include <kernel/proc.h>

void
schedule(void)
{
	struct process *proc;

	for (proc = FIRST_PROC; proc < LAST_PROC; proc++) {
		if (proc == current_process)
			continue;
		if (proc->state == PROC_RUNNABLE) {
			current_process = proc;
			return;
		}
	}
	/* No processes available to run. Resort to idle */
	current_process = FIRST_PROC;
}
