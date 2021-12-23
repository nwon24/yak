/*
 * processes.c
 * Architecture specific code for handling processes.
 */
#include <asm/cpu_state.h>
#include <asm/paging.h>
#include <asm/user_mode.h>

#include <kernel/debug.h>
#include <kernel/proc.h>

#include <mm/mm.h>
#include <mm/vm.h>

extern uint32_t init_page_directory;
/* Page directory for current process */
uint32_t current_page_directory;

/*
 * Allocates user memory for the first process.
 * Arguments passed are the entry point and the size in bytes.
 */
int
arch_processes_init(uint32_t start, uint32_t size)
{
	current_page_directory = virt_map_first_proc(start, size);
	if (!current_page_directory)
		return -1;
	current_cpu_state->cr3 = current_page_directory;
	load_cr3(current_cpu_state->cr3);
	/*
	 * This is really quite ugly...
	 */
	move_to_user(0, 4096);
	return 0;
}
