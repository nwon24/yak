/*
 * kernel/exec_elf.c
 * Generic ELF routines.
 */
#include <stddef.h>

#include <kernel/elf.h>
#include <kernel/exec.h>

#include <kernel/debug.h>

int
elf_verify_hdr(void *hdr)
{
	Elf32_Ehdr *ehdr;

	/*
	 * Magic at the start is same regardless of 32/64 bit
	 * so we can use whichever struct.
	 */
	ehdr = hdr;
	if (ehdr->e_ident[EI_MAG0] != ELFMAG0)
		return -1;
	if (ehdr->e_ident[EI_MAG1] != ELFMAG1)
		return -1;
	if (ehdr->e_ident[EI_MAG2] != ELFMAG2)
		return -1;
	if (ehdr->e_ident[EI_MAG3] != ELFMAG3)
		return -1;
	return 0;
}

int
elf_check_hdr(void *hdr, struct exec_elf_params *param)
{
	Elf32_Ehdr *ehdr32;

	if (param->bits == ELFCLASS32)
		ehdr32 = hdr;
	else
		/* No 64-bit executables yet */
		return -1;
	if (ehdr32->e_ident[EI_CLASS] != param->bits)
		return -1;
	if (ehdr32->e_ident[EI_DATA] != param->endian)
		return -1;
	if (ehdr32->e_ident[EI_OSABI] != param->abi)
		return -1;
	if (ehdr32->e_machine != param->machine)
		return -1;
	return ehdr32->e_type;
}

ssize_t
elf_read(struct exec_elf_file *file, void *buf, ssize_t count, off_t off)
{
	return file->fs->f_driver->fs_readi(file->inode, buf, count, &off);
}
