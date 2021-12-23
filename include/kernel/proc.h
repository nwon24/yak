#ifndef _PROC_H
#define _PROC_H

#define NR_PROC	64

struct process {
	int pid;
	int state;
	int tty;
};

extern struct process *current_process;

void processes_init(void);

#define FIRST_PROC	(&process_table[0])
#define LAST_PROC	(&process_table[NR_PROC])

#define PROC_RUNNING	1
#define PROC_BLOCKED	2
#define PROC_RUNNABLE	3

#endif /* _PROC_H */
