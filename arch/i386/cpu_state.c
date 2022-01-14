/*
 * cpu_state.c
 * Manages the saving of CPU state for a process.
 */
#include <stddef.h>

#include <asm/cpu_state.h>
#include <asm/segment.h>
#include <asm/paging.h>
#include <asm/interrupts.h>

#include <generic/string.h>

#include <kernel/proc.h>
#include <kernel/debug.h>

#include <mm/vm.h>

struct i386_cpu_state *current_cpu_state = NULL;

struct i386_cpu_state cpu_states[NR_PROC];

extern uint32_t boot_stack;

void
cpu_state_init(void)
{
	current_cpu_state = &cpu_states[0];
	current_cpu_state->kernel_stack = (uint32_t)&boot_stack;
	disable_intr();
	tss.ss0 = KERNEL_SS_SELECTOR;
	tss.esp0 = (uint32_t)&boot_stack;
}

void
cpu_state_save(struct i386_cpu_state *new)
{
	/*
	 * Copy just everything up to eflags because the interrupt might not have
	 * happened in userspace, in which case esp and ss are not saved.
	 */
	memmove(current_cpu_state, new, (char *)&new->ss - (char *)&new->ebp);
	if (new->ss == USER_DS_SELECTOR) {
		current_cpu_state->ss = new->ss;
		current_cpu_state->esp = new->esp;
	}
}
