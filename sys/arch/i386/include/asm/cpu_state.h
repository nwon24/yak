#ifndef CPU_STATE_H
#define CPU_STATE_H

#define IRET_FRAME_EBP		0
#define IRET_FRAME_EDI		4
#define IRET_FRAME_ESI		8
#define IRET_FRAME_EDX		12
#define IRET_FRAME_ECX		16
#define IRET_FRAME_EBX		20
#define IRET_FRAME_EAX		24
#define IRET_FRAME_GS		28
#define IRET_FRAME_FS		32
#define IRET_FRAME_ES		36
#define IRET_FRAME_DS		40
#define IRET_FRAME_EIP		48
#define IRET_FRAME_CS		52
#define IRET_FRAME_EFLAGS	56
#define IRET_FRAME_ESP		60
#define IRET_FRAME_SS		64

#define IRET_FRAME_OFF		76

#ifndef __ASSEMBLER__

#include <stdint.h>

#define IRET_FRAME_SIZE	68

/* Is better named 'trap frame' or 'iret frame' */
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
	uint32_t iret_frame;
	uint32_t kernel_eflags;
	uint32_t next_cr3;	/* Value of CR3 for newly exec'ed program */
};

#define NR_REGS	8

struct context {
	uint32_t regs[NR_REGS];
	uint32_t eip;
};

void cpu_state_init(void);
void cpu_state_save(struct i386_cpu_state *new);

extern struct i386_cpu_state *current_cpu_state;
extern struct i386_cpu_state cpu_states[];

#endif /* __ASSEMBLER__ */

#define KERNEL_EFLAGS_OFF	80

#endif /* CPU_STATE_H */
