/*
 * exceptions.c
 * Handles exceptions.
 * Mostly just for debugging.
 */
#include <stdint.h>

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
do_div_by_zero(uint32_t eip, uint32_t error)
{
	handle_exception("Divide by zero", eip, error);
}

void
do_debug(uint32_t eip, uint32_t error)
{
	handle_exception("Debug", eip, error);
}

void
do_nmi(uint32_t eip, uint32_t error)
{
	handle_exception("Non-maskable interrupt", eip, error);
}

void
do_breakpoint(uint32_t eip, uint32_t error)
{
	handle_exception("Breakpoint", eip, error);
}

void
do_overflow(uint32_t eip, uint32_t error)
{
	handle_exception("Overflow", eip, error);
}

void
do_bound_range_exceeded(uint32_t eip, uint32_t error)
{
	handle_exception("Bound range exceeded", eip, error);
}

void
do_invalid_opcode(uint32_t eip, uint32_t error)
{
	handle_exception("Invalid opcode", eip, error);
}

void
do_dev_not_available(uint32_t eip, uint32_t error)
{
	handle_exception("Device not available", eip, error);
}

void
do_double_fault(uint32_t eip, uint32_t error)
{
	handle_exception("Double fault", eip, error);
}

void
do_invalid_tss(uint32_t eip, uint32_t error)
{
	handle_exception("Invalid TSS", eip, error);
}

void
do_segment_not_present(uint32_t eip, uint32_t error)
{
	handle_exception("Segment not present", eip, error);
}

void
do_stack_segment_fault(uint32_t eip, uint32_t error)
{
	handle_exception("Stack segment fault", eip, error);
}

void
do_gp_fault(uint32_t eip, uint32_t error)
{
	handle_exception("General protection fault", eip, error);
}

void
do_page_fault(uint32_t eip, uint32_t error)
{
	handle_exception("Page fault", eip, error);
}

void
do_x87_floating_point(uint32_t eip, uint32_t error)
{
	handle_exception("x87 floating point exception", eip, error);
}

void
do_alignment_check(uint32_t eip, uint32_t error)
{
	handle_exception("Alignment check", eip, error);
}

void
do_machine_check(uint32_t eip, uint32_t error)
{
	handle_exception("Machine check", eip, error);
}

void
do_simd_floating_point(uint32_t eip, uint32_t error)
{
	handle_exception("SIMD Floating Point Exception", eip, error);
}

void
do_virtualization(uint32_t eip, uint32_t error)
{
	handle_exception("Virtualization Exception", eip, error);
}

void
do_control_protection(uint32_t eip, uint32_t error)
{
	handle_exception("Control Protection Exception", eip, error);
}

void
do_reserved(uint32_t eip, uint32_t error)
{
	handle_exception("Reserved", eip, error);
}
