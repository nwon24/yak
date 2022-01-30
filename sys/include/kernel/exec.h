#ifndef EXEC_H
#define EXEC_H

#include <stdint.h>

#include <fs/fs.h>

#include <kernel/elf.h>

struct exec_image {
	uintptr_t e_entry;
	uintptr_t e_text_vaddr;
	uintptr_t e_text_size;
	uintptr_t e_data_vaddr;
	uintptr_t e_data_size;
	uintptr_t e_rodata_vaddr;
	uintptr_t e_rodata_size;
	uintptr_t e_bss_vaddr;
	uintptr_t e_bss_size;
};

struct exec_elf_params {
	uint8_t endian;
	uint8_t bits;
	uint8_t abi;
	Elf32_Half machine;
};

ssize_t elf_read(struct exec_elf_file *file, void *buf, ssize_t count, off_t off);

#endif /* EXEC_H */
