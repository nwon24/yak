/*
 * boot.S
 * Assembly bootstrap code.
 */

#include <generic/multiboot.h>
#include <asm/segment.h>
#include <asm/paging.h>

/* Constants for multiboot header. */
#define MB_MAGIC	MULTIBOOT_HEADER_MAGIC
#define MB_FLAGS	(MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO | MULTIBOOT_VIDEO_MODE)
#define MB_CHECKSUM	-(MB_MAGIC + MB_FLAGS)

/* Size of initial kernel stack. */
#define BOOT_STACK_SIZE	16384

	.section .multiboot_header, "aw"
	.align	4
multiboot_header:
	.long	MB_MAGIC
	.long	MB_FLAGS
	.long	MB_CHECKSUM
	.fill	9, 4, 0

	.section .bss, "aw", @nobits
	.align	16
	.space	BOOT_STACK_SIZE
boot_stack:
	.align	PAGE_SIZE
init_page_directory:
	.space	PAGE_SIZE
init_page_table:
	.space	PAGE_SIZE

	/*
	 * 'low_text' section has physical address in low memory, so it can actually run
	 * before paging is enabled.
	 */
	.section .low_text, "a"
	.global	_start
	.extern	kernel_main, gdt_init, _init
	.type	_start, @function
_start:
	movl	$ASM_PHYS_ADDR(boot_stack), %esp	/* Set up stack */

	/*
	 * Save %ebx and %eax (with multiboot struct and magic)
	 * Change the multiboot struct to its virtual address.
	 */
	addl	$KERNEL_VIRT_BASE, %ebx
	movl	%eax, ASM_PHYS_ADDR(_mb_magic_saved)
	movl	%ebx, ASM_PHYS_ADDR(_mb_info_ptr_saved)

	/* First map low memory <1MiB */
	movl	$0, %edx
	movl	$0x100000, %esi
	movl	$(PAGE_PRESENT | PAGE_WRITABLE), %ecx
	call	map_section

	/* Now map sections of our kernel */
	movl	$_start_low_text, %edx
	movl	$_end_low_text, %esi
	movl	$(PAGE_PRESENT), %ecx
	calll	map_section
	movl	$(ASM_PHYS_ADDR(_start_text)), %edx
	movl	$(ASM_PHYS_ADDR(_end_text)), %esi
	movl	$PAGE_PRESENT, %ecx
	calll	map_section
	movl	$(ASM_PHYS_ADDR(_start_rodata)), %edx
	movl	$(ASM_PHYS_ADDR(_end_rodata)), %esi
	movl	$PAGE_PRESENT, %ecx
	calll	map_section
	movl	$(ASM_PHYS_ADDR(_start_data)), %edx
	movl	$(ASM_PHYS_ADDR(_end_data)), %esi
	movl	$(PAGE_PRESENT | PAGE_WRITABLE), %ecx
	calll	map_section
	movl	$(ASM_PHYS_ADDR(_start_bss)), %edx
	movl	$(ASM_PHYS_ADDR(_end_bss)), %esi
	movl	$(PAGE_PRESENT | PAGE_WRITABLE), %ecx
	calll	map_section
	movl	$(ASM_PHYS_ADDR(init_page_table)), %eax
	orl	$(PAGE_PRESENT | PAGE_WRITABLE | PAGE_SUPERVISOR), %eax
	movl	$(ASM_PHYS_ADDR(init_page_directory)), %edi
	movl	%eax, (%edi)
	movl	$KERNEL_VIRT_BASE, %ecx
	shrl	$VIRT_ADDR_PG_DIR_SHIFT, %ecx
	/* Multiply by 4 as that is the size of each entry in the page directory */
	shll	$2, %ecx
	addl	%ecx, %edi
	movl	%eax, (%edi)
	movl	$(ASM_PHYS_ADDR(init_page_directory)), %eax
	movl	%eax, %cr3
	movl	%cr0, %eax
	orl	$PG_ENABLE, %eax
	movl	%eax, %cr0
	leal	higher_half, %eax
	jmpl	*%eax

	/*
	 * Dispense with the x86 calling convention here.
	 * Maps a section of the kernel into corresponding high memory.
	 * %edx contains starting physical address of section.
	 * %esi contains ending physical address of section.
	 * %ecx contains flags for current section.
	 */
map_section:
	movl	$(ASM_PHYS_ADDR(init_page_table)), %edi
	movl	%edx, %eax
	shrl	$PAGE_SHIFT-2, %edx	/* Subtract 2 because size of each page table entry is 2^2=4 bytes */
	addl	%edx, %edi
1:	cmpl	%esi, %eax
	jge	2f
	movl	%eax, %ebx
	orl	%ecx, %ebx
	movl	%ebx, (%edi)
	addl	$PAGE_SIZE, %eax
	addl	$4, %edi
	jmp	1b
2:	ret

	.section .text
higher_half:
	/*
	 * First, set stack to its proper virtual address.
	 * Must be done before identity mapping is removed, as that would cause a page fault.
	 */
	movl	$boot_stack, %esp
	/* Remove identity mapping */
	movl	$0, init_page_directory
	/* Reload cr3 to force TLB flush */
	movl	%cr3, %eax
	movl	%eax, %cr3
	calll	_init
	/* Initialise and load GDT */
	calll	gdt_init
	lgdtl	gdt_desc
	ljmpl	$KERNEL_CS_SELECTOR, $1f
1:	movl	$KERNEL_DS_SELECTOR, %eax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %ss
	movw	%ax, %gs
	pushl	_mb_magic_saved
	pushl	_mb_info_ptr_saved
	calll	kernel_main
	cli
1:	hlt
	jmp	1b

	.section .data
gdt_desc:
	.word	GDT_ENTRIES * GDT_ENTRY_SIZE - 1
	.long	gdt
_mb_magic_saved:
	.long	0
_mb_info_ptr_saved:
	.long	0
