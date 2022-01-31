/*
 * processes.c
 * Architecture specific code for handling processes.
 * Contains code used in 'fork' and 'exec'
 */
#include <asm/cpu_state.h>
#include <asm/paging.h>
#include <asm/user_mode.h>
#include <asm/idt.h>
#include <asm/syscall.h>
#include <asm/segment.h>
#include <asm/interrupts.h>
#include <asm/uaccess.h>

#include <drivers/tty.h>
#include <drivers/timer.h>

#include <generic/errno.h>
#include <generic/string.h>
#include <generic/unistd.h>

#include <fs/fs.h>

#include <kernel/elf.h>
#include <kernel/exec.h>
#include <kernel/debug.h>
#include <kernel/proc.h>

#include <mm/mm.h>
#include <mm/vm.h>

#define MAX_ARG_PAGES	32

int elf32_read_ehdr(struct exec_elf_file *file, Elf32_Ehdr *hdr);
int elf_check_hdr(void *hdr, struct exec_elf_params *param);
int elf32_load_elf(struct exec_image *image, struct exec_elf_file *file, Elf32_Ehdr *hdr);
int elf32_read_image(struct exec_image *image, struct exec_elf_file *file);

static size_t exec_count_str(const char **str);
static size_t exec_strlen(const char *s);
static void *exec_set_up_stack(const char *argv[], const char *envp[], size_t *res_stack_gap);

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
	register_syscall(__NR_fork, (size_t)kernel_fork, 0);
	register_syscall(0, (size_t)kernel_test, 1);
	register_syscall(__NR_exit, (size_t)kernel_exit, 1);
	register_syscall(__NR_execve, (size_t)kernel_execve, 3);
	current_page_directory = virt_map_chunk(start, size, NULL, PAGE_WRITABLE | PAGE_USER, start);
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
	move_to_user(start, start + PAGE_SIZE);
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
	new->kernel_stack = (uint32_t)kvmalloc(PAGE_SIZE * 2) + (PAGE_SIZE * 2);
	if ((new->cr3 = page_frame_alloc()) == NO_FREE_PAGE)
		return -ENOMEM;
	copy_address_space(current_cpu_state->cr3, new->cr3);
	new->kernel_stack -= IRET_FRAME_SIZE;
	memmove((void *)new->kernel_stack, new, IRET_FRAME_SIZE);
	new->kernel_stack -= sizeof(*proc->context);
	proc->context = (struct context *)new->kernel_stack;
	memset(proc->context, 1, sizeof(uint32_t) * NR_REGS);
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
arch_exit(void)
{
	free_address_space(&current_process->image);
}

int
arch_exec_elf(struct exec_elf_file *file, const char *argv[], const char *envp[])
{
	struct exec_elf_params param;
	struct exec_image image;
	int type, err;
	size_t init_stack_gap;
	uint32_t *new_sp;
	void *padded_sp;
	Elf32_Ehdr ehdr;

	if (elf32_read_ehdr(file, &ehdr) != sizeof(ehdr))
		return -ENOEXEC;
	if (elf_verify_hdr(&ehdr) < 0)
		return -ENOEXEC;
	param.endian = ELFDATA2LSB;
	param.bits = ELFCLASS32;
	param.abi = ELFABI_SYSTEMV;
	param.machine = EM_X86;
	type = elf_check_hdr(&ehdr, &param);
	if (type < 0)
		return -ENOEXEC;
	if (type != ET_EXEC)
		return -ENOEXEC;
	err = elf32_load_elf(&image, file, &ehdr);
	init_stack_gap = 0;
	new_sp = exec_set_up_stack(argv, envp, &init_stack_gap);
	padded_sp = (void *)(KERNEL_VIRT_BASE - init_stack_gap);
	((struct i386_cpu_state *)current_cpu_state->iret_frame)->esp = (uint32_t)padded_sp;
	((struct i386_cpu_state *)current_cpu_state->iret_frame)->eip = image.e_entry;
	/* free_address_space(current_process->image); */
	copy_page_table((uint32_t *)current_cpu_state->cr3, (uint32_t *)current_cpu_state->next_cr3);
	page_frame_free((uint32_t)current_cpu_state->next_cr3);
	memmove(padded_sp, new_sp, init_stack_gap);
	elf32_read_image(&image, file);
	if (err < 0)
		return err;
	return 0;
}

/*
 * Not easy to understand.
 * This function basically sets up the argv and envp tables in user memory.
 */
static void *
exec_set_up_stack(const char *argv[], const char *envp[], size_t *res_stack_gap)
{
	char *sp;
	uint32_t *roving, run, user_sp_base;
	const char **p;
	size_t argv_count, envp_count;
	size_t total_len, i, j;
	uint8_t c;
	int argc;

	total_len = 0;
	argv_count = exec_count_str(argv);
	/*
	 * -1 because 'argv_count' counts NULL, i.e.,
	 *  { "/bin/foo", NULL } --> argc = 1
	 */
	argc = argv_count - 1;
	total_len += argv_count * sizeof(void *);
	envp_count = exec_count_str(envp);
	total_len += envp_count * sizeof(void *);
	for (p = argv; *p != NULL; p++)
		total_len += exec_strlen(*p);
	for (p = envp; *p != NULL; p++)
		total_len += exec_strlen(*p);
	/* Size of argc, argv, and envp */
	total_len += 3 * sizeof(void *);
	user_sp_base = KERNEL_VIRT_BASE - total_len;
	user_sp_base &= 0xFFFFFFF0;
	*res_stack_gap = KERNEL_VIRT_BASE - user_sp_base;
	sp = kvmalloc(total_len);
	/*
	 * Perhaps this might help.
	 * Value of sp that pgoram begins with is here.
	 * | argc
	 * | Pointer to first doubleword after this pointer and 'envp' pointer (argv)
	 * | Pointer to first doubleword after this pointer and string pointer in argv (envp)
	 * | Start of string pointers in argv (pointer to after end of argv and envp string pointers)
	 * | ...
	 * | Start of string pointers in envp (pointer to after envp string pointers)
	 * | ...
	 * | Strings themselves (first argv, then envp)
	 * V KERNEL_VIRT_BASE
	 */
	sp -= 12;
	roving = (uint32_t *)sp;
	*roving = argc;
	roving++;
	*roving = (uint32_t)(user_sp_base + 3 * sizeof(void *));
	roving++;
	*roving = (uint32_t)user_sp_base + 3 * sizeof(void *) + argv_count * sizeof(void *);
	roving++;
	run = (uint32_t)(sp + 12 + argv_count * sizeof(void *) + envp_count * sizeof(void *));
	for (i = 0; i < argv_count; i++) {
		size_t tmp;

		if (argv[i] != NULL) {
			*roving = user_sp_base + run - (uint32_t)sp;
			tmp = exec_strlen(argv[i]);
			for (j = 0; j < tmp; j++) {
				c = get_ubyte(argv[i] + j);
				*(char *)(run + j) = c;
			}
			run += tmp;
		} else {
			*roving = 0;
		}
		roving++;
	}
	for (i = 0; i < envp_count; i++) {
		size_t tmp;

		if (envp[i] != NULL) {
			*roving = user_sp_base + run - (uint32_t)sp;
			tmp = exec_strlen(envp[i]);
			for (j = 0; j < tmp; j++) {
				c = get_ubyte(envp[i] + j);
				*(char *)(run + j) = c;
			}
			run += tmp;
		} else {
			*roving = 0;
		}
		roving++;
	}
	printk("argv_count %d envp_count %d total len %u\r\n", argv_count, envp_count, total_len);
	return sp;
}

static size_t
exec_count_str(const char **str)
{
	const char **p;

	for (p = str; (void *)get_ulong((void *)p) != NULL; p++);
	return p - str + 1;
}

/*
 * Like the regular 'strlen' except it includes the terminating '\0'
 */
static size_t
exec_strlen(const char *s)
{
	size_t len;
	const char *p;

	for (len = 0, p = s; get_ubyte(p) != '\0'; p++)
		len++;
	return len + 1;
}

void
kernel_test(int n)
{
	if (n)
		tty_write(0, "1", 1);
	else
		tty_write(0, "0", 1);
}
