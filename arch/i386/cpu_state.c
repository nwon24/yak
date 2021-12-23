/*
 * cpu_state.c
 * Manages the saving of CPU state for a process.
 */
#include <stddef.h>

#include <asm/cpu_state.h>
#include <asm/segment.h>
#include <asm/paging.h>

#include <generic/string.h>

#include <kernel/proc.h>
#include <kernel/debug.h>

struct i386_cpu_state *current_cpu_state = NULL;

static struct i386_cpu_state cpu_states[NR_PROC];

extern uint32_t boot_stack;

void
cpu_state_init(void)
{
	current_cpu_state = &cpu_states[0];
	current_cpu_state->kernel_stack = (uint32_t)&boot_stack;
	tss.ss0 = KERNEL_SS_SELECTOR;
	tss.esp0 = (uint32_t)&boot_stack;
}

struct i386_cpu_state *
cpu_state_save(struct i386_cpu_state *new)
{
	memmove(current_cpu_state, new, sizeof(*new) - (sizeof(new->cr3) + sizeof(new->kernel_stack)));
	return current_cpu_state;
}
