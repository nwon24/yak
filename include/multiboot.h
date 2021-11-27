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

#ifndef _ASSEMBLY_

#include <stdint.h>

#endif /* _ASSEMBLY_ */
#endif /* _MULTIBOOT_H */
