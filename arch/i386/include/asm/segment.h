#ifndef _SEGMENT_H
#define _SEGMENT_H

/* Size of a GDT entry */
#define GDT_ENTRY_SIZE	8

/*
 * Number of GDT entries.
 * TODO: Increase for TSS.
 */
#define GDT_ENTRIES	5

/* GDT entry types. */

/*
 * Some of theses names are long, but are not used
 * since the processor, not the software, sets the accessed bit.
 */

/* Data, read-only */
#define SEG_TYPE_DATA_RO		0x0
/* Data, read-only, accessed. */
#define SEG_TYPE_DATA_RO_ACCESSED	0x1
/* Data, read-write */
#define SEG_TYPE_DATA_RW		0x2
/* Data, read-write, accessed. */
#define SEG_TYPE_DATA_RW_ACCESSED	0x3
/* Data, read-only, expand-down */
#define SEG_TYPE_DATA_RO_EXP_DOWN	0x4
/* Data, read-only, expand-down, accessed */
#define SEG_TYPE_DATA_RO_EXP_DOWN_ACCESSED	0x5
/* Data, read-write, expand-down */
#define SEG_TYPE_DATA_RW_EXP_DOWN	0x6
/* Data, read-write, expand-down, accessed. */
#define SEG_TYPE_DATA_RW_EXP_DOWN_ACCESSED	0x7

/* Code, execute-only */
#define SEG_TYPE_CODE_XO		0x8
/* Code, execute-only, accessed. */
#define SEG_TYPE_CODE_XO_ACCESSED	0x9
/* Code, execute-read */
#define SEG_TYPE_CODE_RX		0xA
/* Code, execute-read, accessed. */
#define SEG_TYPE_CODE_RX_ACCESSED	0xB
/* Code, execute-only, conforming */
#define SEG_TYPE_CODE_XO_CONFORMING	0xC
/* Code, execute-only, conforming, accessed */
#define SEG_TYPE_CODE_XO_CONFORMING_ACCESSSED	0xD
/* Code, execute-read, conforming */
#define SEG_TYPE_CODE_RX_CONFORMING	0xE
/* Code, execure-read, conforming, accessed */
#define SEG_TYPE_CODE_RX_CONFORMING_ACCESSED	0xF

/* These flags form high 4 bits above type field */
#define DESC_TYPE_SYSTEM	0
#define DESC_TYPE_CODE_DATA	0x10

#define DPL_3			3
#define DPL_0			0

#define SEG_PRESENT		0x80

/* Bits of the attribute field */
#define SEG_64_BIT		0x2
#define SEG_16_BIT		0
#define SEG_32_BIT		0x4
#define SEG_GRANULARITY		0x8

#define KERNEL_CS_ENTRY		1
#define KERNEL_DS_ENTRY		2
#define USER_CS_ENTRY		3
#define USER_DS_ENTRY		4

#define KERNEL_CS_SELECTOR	(KERNEL_CS_ENTRY << 3)
#define KERNEL_DS_SELECTOR	(KERNEL_DS_ENTRY << 3)
#define USER_CS_SELECTOR	((USER_CS_ENTRY << 3) | DPL_3)
#define USER_DS_SELECTOR	((USER_DS_ENTRY << 3) | DPL_3)

#ifndef _ASSEMBLY_

#ifdef _KERNEL_
#include <stdint.h>
#endif /* _KERNEL_ */

extern uint8_t gdt[];

enum desc_table {
	GDT,
	LDT
};

uint32_t *set_dt_entry(enum desc_table table,
		       int num,
		       uint32_t limit,
		       uint32_t base,
		       uint8_t dpl,
		       uint8_t type,
		       uint8_t attr);
void gdt_init(void);

#endif /* _ASSEMBLY_ */

#endif /* _SEGMENT_H */
