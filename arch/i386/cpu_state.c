/*
 * cpu_state.c
 * Manages the saving of CPU state for a process.
 */
#include <stddef.h>

#include <asm/cpu_state.h>
#include <asm/segment.h>

#include <generic/string.h>

#include <kernel/proc.h>

static struct i386_cpu_state *current_state = NULL;

static struct i386_cpu_state cpu_states[NR_PROC];

extern uint32_t boot_stack;

void
cpu_state_init(void)
{
	current_state = &cpu_states[0];
	tss.ss0 = KERNEL_DS_ENTRY;
	tss.esp0 = boot_stack;
}

struct i386_cpu_state *
cpu_state_save(struct i386_cpu_state *new)
{
	memmove(current_state, new, sizeof(*new));
	return current_state;
}
