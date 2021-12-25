/*
 * processes.c
 * Architecture specific code for handling processes.
 */
#include <asm/cpu_state.h>
#include <asm/paging.h>
#include <asm/user_mode.h>
#include <asm/idt.h>
#include <asm/syscall.h>
#include <asm/segment.h>
#include <asm/interrupts.h>

#include <drivers/tty.h>

#include <generic/errno.h>
#include <generic/string.h>
#include <generic/unistd.h>

#include <kernel/debug.h>
#include <kernel/proc.h>

#include <mm/mm.h>
#include <mm/vm.h>

void syscall(void);

void asm_switch_to(void);

extern uint32_t init_page_directory;
/* Page directory for current process */
uint32_t current_page_directory;

void __kernel_test(int);

/*
 * Allocates user memory for the first process.
 * Arguments passed are the entry point and the size in bytes.
 */
int
arch_processes_init(uint32_t start, uint32_t size)
{
/*	set_idt_entry(SYSCALL_IRQ, (uint32_t)syscall, KERNEL_CS_SELECTOR, DPL_3, IDT_32BIT_TRAP_GATE); */
	set_idt_entry(SYSCALL_IRQ, (uint32_t)syscall, KERNEL_CS_SELECTOR, DPL_3, IDT_32BIT_INT_GATE);
	register_syscall(__NR_fork, (uint32_t)__kernel_fork, 0);
	register_syscall(0, (uint32_t)__kernel_test, 1);
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

/*
 * Architecture specifics of fork routine.
 * Integer passed is PID of child process.
 */
int
arch_fork(int child)
{
	struct i386_cpu_state *new;

	if ((new = cpu_states + child) >= cpu_states + NR_PROC)
		return -EAGAIN;
	memmove(new, current_cpu_state, sizeof(*new));
	/* Return value for child process is 0 */
	new->eax = 0;
	new->kernel_stack = (uint32_t)kvmalloc(PAGE_SIZE) + PAGE_SIZE - 1;
	if ((new->cr3 = page_frame_alloc()) == NO_FREE_PAGE)
		return -ENOMEM;
	copy_address_space(current_cpu_state->cr3, new->cr3);
	return child;
}

/*
 * Should switch to a different process.
 */
void
arch_switch_to(int state)
{
	disable_intr();
	current_cpu_state = cpu_states + state;
/*	if ((current_cpu_state->cr3 & 0xFFF) || !current_cpu_state->cr3)
		__asm__("cli; hlt"); */
	load_cr3(current_cpu_state->cr3);
	tss.esp0 = current_cpu_state->kernel_stack;
	asm_switch_to();
}

void
__kernel_test(int n)
{
	if (n)
		tty_write(0, "1", 1);
	else
		tty_write(0, "0", 1);
}
