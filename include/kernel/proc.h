#ifndef _PROC_H
#define _PROC_H

#define NR_PROC	64

struct proc_image {
	uint32_t vir_code_base;	/* Only readable */
	uint32_t vir_code_len;
	uint32_t vir_data_base; /* Readable and writeable */
	uint32_t vir_data_len;
};

struct process {
	int pid;
	int state;
	int tty;

	struct proc_image image;
};

extern struct process *current_process;

void processes_init(void);
int __kernel_fork(void);

#define FIRST_PROC	(&process_table[0])
#define LAST_PROC	(&process_table[NR_PROC])

#define PROC_RUNNING	1
#define PROC_BLOCKED	2
#define PROC_RUNNABLE	3

#endif /* _PROC_H */
