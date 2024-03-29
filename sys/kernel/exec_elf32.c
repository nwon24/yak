/*
 * kernel/exec_elf32.c
 * ELF routines for 32-bit.
 */
#include <asm/uaccess.h>
#include <asm/paging.h>

#include <fs/fs.h>

#include <kernel/elf.h>
#include <kernel/exec.h>
#include <kernel/debug.h>
#include <kernel/proc.h>

#include <generic/errno.h>

int
elf32_read_ehdr(struct exec_elf_file *file, Elf32_Ehdr *hdr)
{
	return elf_read(file, hdr, sizeof(*hdr), 0);
}

int
elf32_read_phdr(struct exec_elf_file *file, Elf32_Ehdr *ehdr, Elf32_Phdr *phdr, int entry)
{
	return elf_read(file, phdr, ehdr->e_phentsize, entry * ehdr->e_phentsize + ehdr->e_phoff);
}

int
elf32_load_elf(struct exec_image *image, struct exec_elf_file *file, Elf32_Ehdr *ehdr)
{
	Elf32_Phdr phdr;
	Elf32_Half phnum;

	image->e_entry = ehdr->e_entry;
	image->e_text_vaddr = image->e_text_size = 0;
	image->e_data_vaddr = image->e_data_size = 0;
	image->e_rodata_vaddr = image->e_rodata_size = 0;
	image->e_bss_vaddr = image->e_bss_size = 0;
	for (phnum = 0; phnum < ehdr->e_phnum; phnum++) {
		elf32_read_phdr(file, ehdr, &phdr, phnum);
		if (phdr.p_type == PT_LOAD) {
			if ((phdr.p_flags & PF_X) && (phdr.p_flags & PF_R)) {
				image->e_text_vaddr = phdr.p_vaddr;
				image->e_text_size = phdr.p_memsz;
				file->text_off = phdr.p_offset;
			} else if ((phdr.p_flags & PF_R) && !(phdr.p_flags & PF_W)) {
				image->e_rodata_vaddr = phdr.p_vaddr;
				image->e_rodata_size = phdr.p_memsz;
				file->rodata_off = phdr.p_offset;
			} else if ((phdr.p_flags & PF_R) && (phdr.p_flags & PF_W)) {
				image->e_data_vaddr = phdr.p_vaddr;
				image->e_data_size = phdr.p_memsz;
				if (phdr.p_memsz > phdr.p_filesz) {
					image->e_bss_vaddr = image->e_data_vaddr + phdr.p_filesz;
					image->e_bss_size = phdr.p_memsz - phdr.p_filesz;
				}
			        file->data_off = phdr.p_offset;
			}
		}
	}
	if (arch_valloc_segments(image) < 0)
		return -ENOMEM;
	return 0;
}

int
elf32_read_image(struct exec_image *image, struct exec_elf_file *file)
{
	size_t i;

	if (image->e_text_size)
		elf_read(file, (void *)image->e_text_vaddr, image->e_text_size, file->text_off);
	if (image->e_data_size)
		elf_read(file, (void *)image->e_data_vaddr, image->e_data_size - image->e_bss_size, file->data_off);
	if (image->e_rodata_size)
		elf_read(file, (void *)image->e_rodata_vaddr, image->e_rodata_size, file->rodata_off);
	if (image->e_bss_size) {
		for (i = 0; i < image->e_bss_size; i++)
			put_ubyte(image->e_bss_vaddr, 0);
	}
	return 0;
}
