/*
 * kernel.ld
 * Linkerscript for x86 kernel.
 */

#include <kernel/config.h>
#include <asm/paging.h>

ENTRY(_start)

SECTIONS
{
	. = KERNEL_LOAD_PHYS_ADDR;

	_start_kernel = .;
	.multiboot_header : ALIGN(PAGE_SIZE) {
		_start_multiboot_header = .;
		*(.multiboot_header)
		_end_multiboot_header = .;
	}

	.user_head : ALIGN(PAGE_SIZE) {
		_start_user_head = .;
		*(.user_head)
		_end_user_head = .;
	}

	.low_text : ALIGN(PAGE_SIZE) {
		_start_low_text = .;
		*(.low_text)
		_end_low_text = .;
	}

	. += KERNEL_VIRT_BASE;

	.text ALIGN(PAGE_SIZE) : AT(ADDR(.text) - KERNEL_VIRT_BASE) {
		_start_text = .;
		*(.text)
		_end_text = .;
	}

	.rodata ALIGN(PAGE_SIZE) : AT(ADDR(.rodata) - KERNEL_VIRT_BASE) {
		_start_rodata = .;
		*(.rodata)
		_end_rodata = .;
	}

	.data ALIGN(PAGE_SIZE) : AT(ADDR(.data) - KERNEL_VIRT_BASE) {
		_start_data = .;
		*(.data)
		_end_data = .;
	}

	.bss ALIGN(PAGE_SIZE) : AT(ADDR(.bss) - KERNEL_VIRT_BASE) {
		_start_bss = .;
		*(COMMON)
		*(.bss)
		_end_bss = .;
	}
	. = ALIGN(PAGE_SIZE);
	_end_kernel = .;
}
