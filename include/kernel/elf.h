#ifndef ELF_H
#define ELF_H

#include <stdint.h>

/*
 * Strange ELF types.
 */
typedef uint16_t Elf32_Half;
typedef uint32_t Elf32_Off;
typedef uint32_t Elf32_Addr;
typedef uint32_t Elf32_Word;
typedef int32_t Elf32_Sword;

#define ELF_NIDENT	16

enum ELF_Ident {
	EI_MAG0,
	EI_MAG1,
	EI_MAG2,
	EI_MAG3,
	EI_CLASS,
	EI_DATA,
	EI_VERSION,
	EI_OSABI,
	EI_PAD,
};

#define ELFMAG0		0x7F
#define ELFMAG1		'E'
#define ELFMAG2		'L'
#define ELFMAG3		'F'

#define ELFDATA2LSB	1
#define ELFDATA2MSB	2

#define ELFCLASS32	1
#define ELFCLASS64	2

#define ET_NONE	0
#define ET_REL	1
#define ET_EXEC	2
#define ET_SYN	3
#define ET_CORE	4

#define EV_NONE		0
#define EV_CURRENT	1
#define EV_NUM		2

#define EM_NONE		0
#define EM_SPARC	1
#define EM_X86		3
#define EM_MIPS		8
#define EM_POWERPC	0x14
#define EM_ARM		0x28
#define EM_SUPERH	0x2A
#define EM_IA64		0x32
#define EM_X86_64	0x3E
#define EM_AARCH64	0xB7
#define EM_RISCV	0xF3

#define ELFABI_SYSTEMV	0

#define PT_NULL		0
#define PT_LOAD		1
#define PT_DYNAMIC	2
#define PT_INTERP	3
#define PT_NOTE		4
#define PT_SHLIB	5
#define PT_PHDR		6
#define PT_TLS		7

#define PF_X		1
#define PF_W		2
#define PF_R		4

typedef struct {
	uint8_t e_ident[ELF_NIDENT];
	Elf32_Half e_type;
	Elf32_Half e_machine;
	Elf32_Word e_version;
	Elf32_Addr e_entry;
	Elf32_Off e_phoff;
	Elf32_Off e_shoff;
	Elf32_Word e_flags;
	Elf32_Half e_ehsize;
	Elf32_Half e_phentsize;
	Elf32_Half e_phnum;
	Elf32_Half e_shentsize;
	Elf32_Half e_shnum;
	Elf32_Half e_shstrndx;
} Elf32_Ehdr;

typedef struct {
	Elf32_Word sh_name;
	Elf32_Word sh_type;
	Elf32_Word sh_flags;
	Elf32_Addr sh_addr;
	Elf32_Off sh_offset;
	Elf32_Word sh_size;
	Elf32_Word sh_link;
	Elf32_Word sh_ino;
	Elf32_Word sh_addralign;
	Elf32_Word sh_entsize;
} Elf32_Shdr;

typedef struct {
	Elf32_Word p_type;
	Elf32_Off p_offset;
	Elf32_Addr p_vaddr;
	Elf32_Addr p_paddr;
	Elf32_Word p_filesz;
	Elf32_Word p_memsz;
	Elf32_Word p_flags;
	Elf32_Word p_align;
} Elf32_Phdr;

int elf_verify_hdr(void *hdr);

#endif /* ELF_H */
