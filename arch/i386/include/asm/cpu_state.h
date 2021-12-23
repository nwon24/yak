#ifndef _CPU_STATE_H
#define _CPU_STATE_H

#include <stdint.h>

struct i386_cpu_state {
	uint32_t ebp;
	uint32_t edi;
	uint32_t esi;
	uint32_t edx;
	uint32_t ecx;
	uint32_t ebx;
	uint32_t eax;

	uint32_t gs;
	uint32_t fs;
	uint32_t es;
	uint32_t ds;

	uint32_t error;

	uint32_t eip;
	uint32_t cs;
	uint32_t eflags;
	uint32_t esp;
	uint32_t ss;

	uint32_t cr3;
	uint32_t kernel_stack;
};

void cpu_state_init(void);
struct i386_cpu_state *cpu_state_save(struct i386_cpu_state *new);

extern struct i386_cpu_state *current_cpu_state;

#endif /* _CPU_STATE_H */
