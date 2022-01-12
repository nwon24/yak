/*
 * switch_to.S
 * Context switching code.
 */
#include <asm/cpu_state.h>

/*
 * WARNING: HUge comment.
 * The actual code for the context switching is really quite simple.
 * We don't worry about switching between userspace tasks - rather, we switch
 * between two "contexts" in kernel mode. This consists of the stack pointer,
 * registers, and instruction pointer. When a kernel context calls 'asm_switch_to', the return
 * address is pushed onto the stack. We then save the current context by pushing the general
 * purpose registers, then save a pointer to this context in the pointer to the process's 'context'
 * field. The second parameter, a pointer to the new process's context, is then moved into %esp,
 * which then contains this new context's general purpose registers, and return address. Thus, after
 * a context switch, the process will return to the code just after the call to 'asm_switch_to'.
 * The code to deal with switching between processes happenes just before - %cr3 is loaded with
 * the correct page table and the TSS has the kernel stack pointer.
 */
	.section .text
	.global asm_switch_to
asm_switch_to:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%eax
	movl	8(%ebp), %eax
	movl	%eax, old_context
	movl	12(%ebp), %eax
	movl	%eax, new_context
	popl	%eax
	movl	%ebp, %esp
	popl	%ebp

	pushf
	push	%eax
	push	%ebx
	push	%ecx
	push	%edx
	push	%edi
	push	%esi
	push	%ebp

	movl	%esp, (old_context)
	movl	new_context, %esp

	pop	%ebp
	pop	%esi
	pop	%edi
	pop	%edx
	pop	%ecx
	pop	%ebx
	pop	%eax
	popf

	ret

	.section .data
old_context:
	.long	0
new_context:
	.long	0
