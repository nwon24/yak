/*
 * multiboot.h
 * Multiboot header file.
 * Only includes constants, definitions, etc. that are actually used.
 */

#ifndef _MULTIBOOT_H
#define _MULTIBOOT_H

/* Magic number of multiboot header. */
#define MULTIBOOT_HEADER_MAGIC		0x1BADB002

/* Magic number put in %eax. */
#define MULTIBOOT_BOOTLOADER_MAGIC	0x2BADB002

/* Multiboot flags for multiboot header */

/* Align boot modules on x86 page boundaries (4 KiB) */
#define MULTIBOOT_PAGE_ALIGN		0x00000001

/* Pass memory information. */
#define MULTIBOOT_MEMORY_INFO		0x00000002

/* Set video mode and pass video mode information. */
#define MULTIBOOT_VIDEO_MODE		0x00000004

/* Multiboot flags for flags field of multiboot info struct. */

/* Lower/upper memory information. */
#define MULTIBOOT_INFO_MEMORY		0x00000001

/* Boot device information. */
#define MULTIBOOT_INFO_BOOTDEV		0x00000002

/* Command line */
#define MULTIBOOT_INFO_CMDLINE		0x00000004

/* If there are any modules. */
#define MULTIBOOT_INFO_MODS		0x00000008

/* Symbol table (for a.out) */
#define MULTIBOOT_INFO_AOUT_SYMS	0x00000010

/* ELF section header table. */
#define MULTIBOOT_INFO_ELF_SHDR		0x00000020

/* Memory map. */
#define MULTIBOOT_INFO_MEM_MAP		0x00000040

/* Drive info. */
#define MULTIBOOT_INFO_DRIVE_INFO	0x00000080

/* Config table */
#define MULTIBOOT_INFO_CONFIG_TABLE	0x00000100

/* Bootloader name. */
#define MULTIBOOT_INFO_BOOT_LOADER_NAME	0x00000200

/* APM (Advanced Power Management) table. */
#define MULTIBOOT_INFO_APM_TABLE	0x00000400

/* Video information. */
#define MULTIBOOT_INFO_VBE_INFO		0x00000800

/* Framebuffer information. */
#define MULTIBOOT_INFO_FRAMEBUFFER_INFO	0x00001000

#define MULTIBOOT_FLAG(mb_info, flag)	((mb_info)->flags & (flag))

#ifndef _ASSEMBLY_

#include <stdint.h>

#include <kernel/config.h>

struct multiboot_aout_symbol_table {
	uint32_t tabsize;
	uint32_t strsize;
	uint32_t addr;
	uint32_t reserved;
};
typedef struct multiboot_aout_symbol_table multiboot_aout_symbol_table_t;

struct multiboot_elf_section_header_table {
	uint32_t num;
	uint32_t size;
	uint32_t addr;
	uint32_t shndx;
};
typedef struct multiboot_elf_section_header_table multiboot_elf_section_header_table_t;

struct multiboot_info {
	uint32_t flags;
	uint32_t mem_lower;
	uint32_t mem_upper;
	uint32_t boot_device;
	uint32_t cmdline;
	uint32_t mods_count;
	uint32_t mods_addr;
	union {
		multiboot_aout_symbol_table_t aout_sym;
		multiboot_elf_section_header_table_t elf_sec;
	} syms;
	uint32_t mmap_length;
	uint32_t mmap_addr;
	uint32_t drives_length;
	uint32_t drives_addr;
	uint32_t config_table;
	uint32_t boot_loader_name;
	uint32_t apm_table;
	uint32_t vbe_control_info;
	uint32_t vbe_mode_info;
	uint16_t vbe_mode;
	uint16_t vbe_interface_seg;
	uint16_t vbe_interface_off;
	uint16_t vbe_interface_len;
#ifdef _CONFIG_ARCH_X86
	uint32_t framebuffer_addr_low;
	uint32_t framebuffer_addr_high;
#else
#error "No support for 64 bit"
#endif /* _CONFIG_ARCH_X86 */
	uint32_t framebuffer_pitch;
	uint32_t framebuffer_width;
	uint32_t framebuffer_height;
	uint8_t framebuffer_bpp;
#define MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED	0
#define MULTIBOOT_FRAMEBUFFER_TYPE_RGB		1
#define MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT	2
	uint8_t framebuffer_type;
	union {
		struct {
			uint32_t framebuffer_palette_addr;
			uint16_t framebuffer_palette_num_colors;
		} palette;
		struct {
			uint8_t framebuffer_red_field_position;
			uint8_t framebuffer_red_mask_size;
			uint8_t framebuffer_green_field_positon;
			uint8_t framebuffer_green_mask_size;
			uint8_t framebuffer_blue_field_positon;
			uint8_t framebuffer_blue_mask_size;
		} rgb;
	} color_info;
};
typedef struct multiboot_info multiboot_info_t;

struct multiboot_color {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
};

struct multiboot_mmap_entry {
	uint32_t size;
#ifdef _CONFIG_ARCH_X86
	uint32_t addr_low;
	uint32_t addr_high;
	uint32_t len_low;
	uint32_t len_high;
#else
#error "No support for 64 bit"
#endif /* _CONFIG_ARCH_X86 */
#define MULTIBOOT_MEMORY_AVAILABLE	1
#define MULTIBOOT_MEMORY_RESERVED	2
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE	3
#define MULTIBOOT_MEMORY_NVS		4
#define MULTIBOOT_MEMORY_BADRAM		5
	uint32_t type;
}__attribute__((packed));
typedef struct multiboot_mmap_entry multiboot_mmap_entry_t;

struct multiboot_mod_list {
	uint32_t mod_start;
	uint32_t mod_end;
	uint32_t cmdline;
	uint32_t pad;
};
typedef struct multiboot_mod_list multiboot_module_t;

struct multiboot_apm_info {
	uint16_t version;
	uint16_t cseg;
	uint32_t offset;
	uint16_t cseg_16;
	uint16_t dseg;
	uint16_t flags;
	uint16_t cseg_len;
	uint16_t cseg_16_len;
	uint16_t dseg_len;
};

#endif /* _ASSEMBLY_ */

#endif /* _MULTIBOOT_H */
