/*
 * kernel/exec_elf32.c
 * ELF routines for 32-bit.
 */

#include <fs/fs.h>

#include <kernel/elf.h>
#include <kernel/exec.h>

#include <kernel/debug.h>

int
elf32_read_ehdr(struct exec_elf_file *file, Elf32_Ehdr *hdr)
{
	return elf_read(file, hdr, sizeof(*hdr), 0);
}

int
elf32_load_elf(struct exec_image *image, struct exec_elf_file *file)
{
	Elf32_Ehdr ehdr;

	if (elf32_read_ehdr(file, &ehdr) != sizeof(ehdr))
		return -1;
	if (elf_verify_hdr(&ehdr) < 0)
		return -1;
	printk("elf32_load_elf: number of program headers: %u\r\n", ehdr.e_phnum);
	printk("elf32_load_elf: entry point %x\r\n", ehdr.e_entry);
	printk("elf32_load_elf: program header table offset %u\r\n", ehdr.e_phoff);
	printk("elf32_load_elf: header size %u\r\n", ehdr.e_ehsize);
	return 0;
}
