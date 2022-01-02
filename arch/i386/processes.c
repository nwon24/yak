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
#include <drivers/timer.h>

#include <generic/errno.h>
#include <generic/string.h>
#include <generic/unistd.h>

#include <kernel/debug.h>
#include <kernel/proc.h>

#include <mm/mm.h>
#include <mm/vm.h>

void restart(void);

void syscall(void);

void asm_switch_to(struct context **old, struct context *new);

extern uint32_t init_page_directory;
/* Page directory for current process */
uint32_t current_page_directory;

void kernel_test(int);

/*
 * Allocates user memory for the first process.
 * Arguments passed are the entry point and the size in bytes.
 */
int
arch_processes_init(uint32_t start, uint32_t size)
{
/*	set_idt_entry(SYSCALL_IRQ, (uint32_t)syscall, KERNEL_CS_SELECTOR, DPL_3, IDT_32BIT_TRAP_GATE);  */
	/*
	 * Not a trap gate here since the first few steps of the syscall interrupt handler does some critical things
	 * during which an interrupt must not happen. Interrupts are then enabled before entering the system call.
	 * Each timer tick is 10 ms so none should be missed from the time the syscall interrupt handler is entered
	 * to the point interrupts are enabled.
	 */
	set_idt_entry(SYSCALL_IRQ, (uint32_t)syscall, KERNEL_CS_SELECTOR, DPL_3, IDT_32BIT_INT_GATE);
	register_syscall(__NR_fork, (uint32_t)kernel_fork, 0);
	register_syscall(0, (uint32_t)kernel_test, 1);
	current_page_directory = virt_map_first_proc(start, size);
	if (!current_page_directory)
		return -1;
	current_cpu_state->cr3 = current_page_directory;
	load_cr3(current_cpu_state->cr3);
	current_cpu_state->kernel_stack = (uint32_t)kvmalloc(PAGE_SIZE) + PAGE_SIZE;
	tss.ss = KERNEL_SS_SELECTOR;
	tss.esp0 = current_cpu_state->kernel_stack;
	current_process->context = (struct context *)current_cpu_state->kernel_stack;
	/*
	 * This is really quite ugly...
	 */
	timer_init();
	move_to_user(0, 4096);
	return 0;
}

/*
 * Architecture specifics of fork routine.
 * Integer passed is PID of child process.
 */
int
arch_fork(int child, struct process *proc)
{
	struct i386_cpu_state *new;

	if ((new = cpu_states + child) >= cpu_states + NR_PROC)
		return -EAGAIN;
	memmove(new, current_cpu_state, sizeof(*new));
	/* Return value for child process is 0 */
	new->eax = 0;
	new->kernel_stack = (uint32_t)kvmalloc(PAGE_SIZE) + PAGE_SIZE;
	if ((new->cr3 = page_frame_alloc()) == NO_FREE_PAGE)
		return -ENOMEM;
	copy_address_space(current_cpu_state->cr3, new->cr3);
	new->kernel_stack -= IRET_FRAME_SIZE;
	memmove((void *)new->kernel_stack, new, IRET_FRAME_SIZE);
	new->kernel_stack -= sizeof(*proc->context);
	proc->context = (struct context *)new->kernel_stack;
	memmove(proc->context, 0, sizeof(uint32_t) * NR_REGS);
	proc->context->eip = (uint32_t)restart;
	return child;
}

/*
 * Should switch to a different process.
 */
void
arch_switch_to(struct process *prev, struct process *new)
{
	current_cpu_state = cpu_states + new->pid;
	load_cr3(current_cpu_state->cr3);
	tss.esp0 = current_cpu_state->kernel_stack;
	asm_switch_to(&prev->context, new->context);
}

void
kernel_test(int n)
{
	if (n)
		tty_write(0, "1", 1);
	else
		tty_write(0, "0", 1);
}
