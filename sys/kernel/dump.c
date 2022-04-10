/*
 * dump.c
 * Kernel dump on serial console for debugging.
 */
#include <kernel/config.h>

#ifdef CONFIG_DEBUG

#include <kernel/debug.h>
#include <kernel/proc.h>

extern struct process *process_queues[];

void
debug_dump(void)
{
	struct process **proc, *p;

	printk("----RUNNABLE----\n");
	for (proc = process_queues + PROC_QUANTA - 1; proc > process_queues; proc--) {
		printk("---PRI: %d---\n", proc - process_queues);
		if (*proc == NULL) {
			printk("NULL\n");
		} else {
			p = *proc;
			do {
				printk("PID: %d\n", p->pid);
				p = p->queue_next;
			} while (p != *proc);
		}
	}
	printk("---PROCS---\n");
	for (p = FIRST_PROC; p < LAST_PROC; p++) {
		if (p->state != PROC_EXITED) {
			printk("PID: %d, STATE: %d, SLEPP: %p\r\n", p->pid, p->state, p->sleeping_on);
		}
	}
	printk("---CURRENT---\n");
	printk("PID: %d\n", current_process->pid);
}

#endif
