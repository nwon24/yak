#ifndef _PROC_H
#define _PROC_H

#define NR_PROC	64

struct process {
	int pid;
	int state;
	int tty;
};

extern struct process *current_process;

#endif /* _PROC_H */
