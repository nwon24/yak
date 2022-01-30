/*
 * exceptions.c
 * Handles exceptions.
 * Mostly just for debugging.
 */
#include <stdint.h>

#include <asm/cpu_state.h>
#include <asm/idt.h>
#include <asm/segment.h>

#include <kernel/debug.h>

void div_by_zero(void);
void debug(void);
void nmi(void);
void breakpoint(void);
void overflow(void);
void bound_range_exceeded(void);
void invalid_opcode(void);
void dev_not_available(void);
void double_fault(void);
void invalid_tss(void);
void segment_not_present(void);
void stack_segment_fault(void);
void gp_fault(void);
void page_fault(void);
void x87_floating_point(void);
void alignment_check(void);
void machine_check(void);
void simd_floating_point(void);
void virtualization(void);
void control_protection(void);
void reserved(void);

void
exceptions_init(void)
{
	int i;

	set_idt_entry(0, (uint32_t)div_by_zero, KERNEL_CS_SELECTOR, DPL_0, IDT_32BIT_TRAP_GATE);
	set_idt_entry(1, (uint32_t)debug, KERNEL_CS_SELECTOR, DPL_0, IDT_32BIT_TRAP_GATE);
	set_idt_entry(2, (uint32_t)nmi, KERNEL_CS_SELECTOR, DPL_0, IDT_32BIT_TRAP_GATE);
	set_idt_entry(3, (uint32_t)breakpoint, KERNEL_CS_SELECTOR, DPL_0, IDT_32BIT_TRAP_GATE);
	set_idt_entry(4, (uint32_t)overflow, KERNEL_CS_SELECTOR, DPL_0, IDT_32BIT_TRAP_GATE);
	set_idt_entry(5, (uint32_t)bound_range_exceeded, KERNEL_CS_SELECTOR, DPL_0, IDT_32BIT_TRAP_GATE);
	set_idt_entry(6, (uint32_t)invalid_opcode, KERNEL_CS_SELECTOR, DPL_0, IDT_32BIT_TRAP_GATE);
	set_idt_entry(7, (uint32_t)dev_not_available, KERNEL_CS_SELECTOR, DPL_0, IDT_32BIT_TRAP_GATE);
	set_idt_entry(8, (uint32_t)double_fault, KERNEL_CS_SELECTOR, DPL_0, IDT_32BIT_TRAP_GATE);
	set_idt_entry(10, (uint32_t)invalid_tss, KERNEL_CS_SELECTOR, DPL_0, IDT_32BIT_TRAP_GATE);
	set_idt_entry(11, (uint32_t)segment_not_present, KERNEL_CS_SELECTOR, DPL_0, IDT_32BIT_TRAP_GATE);
	set_idt_entry(12, (uint32_t)stack_segment_fault, KERNEL_CS_SELECTOR, DPL_0, IDT_32BIT_TRAP_GATE);
	set_idt_entry(13, (uint32_t)gp_fault, KERNEL_CS_SELECTOR, DPL_0, IDT_32BIT_TRAP_GATE);
	set_idt_entry(14, (uint32_t)page_fault, KERNEL_CS_SELECTOR, DPL_0, IDT_32BIT_TRAP_GATE);
	set_idt_entry(15, (uint32_t)reserved, KERNEL_CS_SELECTOR, DPL_0, IDT_32BIT_TRAP_GATE);
	set_idt_entry(16, (uint32_t)x87_floating_point, KERNEL_CS_SELECTOR, DPL_0, IDT_32BIT_TRAP_GATE);
	set_idt_entry(17, (uint32_t)alignment_check, KERNEL_CS_SELECTOR, DPL_0, IDT_32BIT_TRAP_GATE);
	set_idt_entry(18, (uint32_t)machine_check, KERNEL_CS_SELECTOR, DPL_0, IDT_32BIT_TRAP_GATE);
	set_idt_entry(19, (uint32_t)simd_floating_point, KERNEL_CS_SELECTOR, DPL_0, IDT_32BIT_TRAP_GATE);
	set_idt_entry(20, (uint32_t)virtualization, KERNEL_CS_SELECTOR, DPL_0, IDT_32BIT_TRAP_GATE);
	set_idt_entry(21, (uint32_t)control_protection, KERNEL_CS_SELECTOR, DPL_0, IDT_32BIT_TRAP_GATE);
	for (i = 22; i <= 31; i++)
		set_idt_entry(i, (uint32_t)reserved, KERNEL_CS_SELECTOR, DPL_0, IDT_32BIT_TRAP_GATE);
}

void
handle_exception(char *msg, uint32_t eip, uint32_t error)
{
	printk("Exception: %s\r\n", msg);
	printk("eip: %x, error: %x\r\n", eip, error);
	panic("");
}

void
do_div_by_zero(void)
{
	handle_exception("Divide by zero", current_cpu_state->eip, current_cpu_state->error);
}

void
do_debug(void)
{
	handle_exception("Debug", current_cpu_state->eip, current_cpu_state->error);
}

void
do_nmi(void)
{
	handle_exception("Non-maskable interrupt", current_cpu_state->eip, current_cpu_state->error);
}

void
do_breakpoint(void)
{
	handle_exception("Breakpoint", current_cpu_state->eip, current_cpu_state->error);
}

void
do_overflow(void)
{
	handle_exception("Overflow", current_cpu_state->eip, current_cpu_state->error);
}

void
do_bound_range_exceeded(void)
{
	handle_exception("Bound range exceeded", current_cpu_state->eip, current_cpu_state->error);
}

void
do_invalid_opcode(void)
{
	handle_exception("Invalid opcode", current_cpu_state->eip, current_cpu_state->error);
}

void
do_dev_not_available(void)
{
	handle_exception("Device not available", current_cpu_state->eip, current_cpu_state->error);
}

void
do_double_fault(void)
{
	handle_exception("Double fault", current_cpu_state->eip, current_cpu_state->error);
}

void
do_invalid_tss(void)
{
	handle_exception("Invalid TSS", current_cpu_state->eip, current_cpu_state->error);
}

void
do_segment_not_present(void)
{
	handle_exception("Segment not present", current_cpu_state->eip, current_cpu_state->error);
}

void
do_stack_segment_fault(void)
{
	handle_exception("Stack segment fault", current_cpu_state->eip, current_cpu_state->error);
}

void
do_gp_fault(void)
{
	handle_exception("General protection fault", current_cpu_state->eip, current_cpu_state->error);
}

void
do_page_fault(void)
{
	handle_exception("Page fault", current_cpu_state->eip, current_cpu_state->error);
}

void
do_x87_floating_point(void)
{
	handle_exception("x87 floating point exception", current_cpu_state->eip, current_cpu_state->error);
}

void
do_alignment_check(void)
{
	handle_exception("Alignment check", current_cpu_state->eip, current_cpu_state->error);
}

void
do_machine_check(void)
{
	handle_exception("Machine check", current_cpu_state->eip, current_cpu_state->error);
}

void
do_simd_floating_point(void)
{
	handle_exception("SIMD Floating Point Exception", current_cpu_state->eip, current_cpu_state->error);
}

void
do_virtualization(void)
{
	handle_exception("Virtualization Exception", current_cpu_state->eip, current_cpu_state->error);
}

void
do_control_protection(void)
{
	handle_exception("Control Protection Exception", current_cpu_state->eip, current_cpu_state->error);
}

void
do_reserved(void)
{
	handle_exception("Reserved", current_cpu_state->eip, current_cpu_state->error);
}
