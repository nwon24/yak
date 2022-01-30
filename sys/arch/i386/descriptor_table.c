/*
 * descriptor_table.c
 * Handles x86 descriptor tables.
 */

#include <stddef.h>

#include <asm/segment.h>

uint8_t gdt[GDT_ENTRIES * GDT_ENTRY_SIZE];

struct tss tss;

/*
 * Put an entry into a descriptor table.
 * @table: GDT or LDT.
 * @num: entry number
 * @dpl: Descriptor Privilege Level
 * @type: See asm/segment.h for types and related attributes. 
 * @attr: Granularity, 16/32bit segment, availabile
 * Since there is no LDT, specifying the LDT is invalid.
 * Returns a pointer to table entry.
 */
uint32_t *
set_dt_entry(enum desc_table table,
	     int num,
	     uint32_t limit,
	     uint32_t base,
	     uint8_t dpl,
	     uint8_t type,
	     uint8_t attr)
{
	uint8_t *p;

	if (table == LDT || num >= GDT_ENTRIES)
		return (NULL);

	p = gdt + num * GDT_ENTRY_SIZE;
	*(uint16_t *)p = (uint16_t) limit & 0xFFFF;
	p += 2;
	*(uint16_t *)p = (uint16_t) base & 0xFFFF;
	p += 2;
	*(uint8_t *)p = (uint8_t) (base >> 16) & 0xFF;
	p++;
	*(uint8_t *)p = (uint8_t) (type | SEG_PRESENT | (dpl << 5));
	p++;
	*(uint8_t *)p = (uint8_t) ((limit >> 16) & 0xF) | (attr << 4);
	p++;
	*(uint8_t *)p = (uint8_t) (base >> 24) & 0xFF;

	return ((uint32_t *)p);
}

/*
 * Initialise GDT by adding appropriate segment descriptors.
 * By making base=0 and limit=4GiB, segmentation is basically removed.
 * Paging is preferred.
 */
void
gdt_init(void)
{
	uint8_t cs_attr, ds_attr;

	/* NULL descriptor is already zeroed. */

	/* Kernel code */
	cs_attr = SEG_GRANULARITY | SEG_32_BIT;
	set_dt_entry(GDT, KERNEL_CS_ENTRY, 0xFFFFFFFF, 0, DPL_0, SEG_TYPE_CODE_RX | DESC_TYPE_CODE_DATA, cs_attr);
	/* Kernel data */
	ds_attr = SEG_GRANULARITY | SEG_32_BIT;
	set_dt_entry(GDT, KERNEL_DS_ENTRY, 0xFFFFFFFF, 0, DPL_0, SEG_TYPE_DATA_RW | DESC_TYPE_CODE_DATA, ds_attr);

	/* User code */
	set_dt_entry(GDT, USER_CS_ENTRY, 0xFFFFFFFF, 0, DPL_3, SEG_TYPE_CODE_RX | DESC_TYPE_CODE_DATA, cs_attr);
	/* User data */
	set_dt_entry(GDT, USER_DS_ENTRY, 0xFFFFFFF, 0, DPL_3, SEG_TYPE_DATA_RW | DESC_TYPE_CODE_DATA, ds_attr);

	/* TSS */
	set_dt_entry(GDT, TSS_ENTRY, sizeof(tss), (uint32_t)&tss, DPL_0, SEG_TYPE_TSS, SEG_32_BIT);
}
