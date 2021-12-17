/*
 * acpi_init.c
 * Detect if ACPI (Advanced Configuration and Power Interface) is available.
 */

#include <stdint.h>
#include <stddef.h>

#include <asm/paging.h>

#include <kernel/debug.h>

#include <generic/string.h>

#define EBDA_ADDR	(*(uint16_t *)VIRT_ADDR(0x40E))

int acpi_supported = 0;

struct rsdp_desc {
        char signature[8];
        uint8_t checksum;
        char oemid[6];
        uint8_t revision;
        uint32_t rsdt_addr;
}__attribute__((packed));

struct acpi_sdt_header {
        char signature[4];
        uint32_t length;
        uint8_t revision;
        uint8_t checksum;
        char oemid[6];
        char oem_table_id[8];
        uint32_t oem_revision;
        uint32_t creator_id;
        uint32_t creator_revision;
}__attribute__((packed));

struct rsdt {
        struct acpi_sdt_header header;
        uint32_t *sdt_pointers;
};

static struct rsdp_desc *find_rsdp(void);
static int acpi_table_check(struct acpi_sdt_header *header);

static struct rsdp_desc *
find_rsdp(void)
{
        uint32_t addr = VIRT_ADDR((EBDA_ADDR << 4));
        uint32_t ptr;

        /* First check the EBDA */
        for (ptr = addr; ptr < addr + 1024; ptr += 16) {
                if (memcmp((void *)ptr, "RSD PTR ", 8) == 0)
                        return (struct rsdp_desc *)ptr;
        }
        for (ptr = VIRT_ADDR(0xE0000); ptr < VIRT_ADDR(0xFFFFF); ptr += 16) {
                if (memcmp((void *)ptr, "RSD PTR ", 8) == 0)
                        return (struct rsdp_desc *)ptr;
        }
        return NULL;
}

static int
acpi_table_check(struct acpi_sdt_header *header)
{
        unsigned char sum = 0;
        uint32_t i;

        for (i = 0; i < header->length; i++)
                sum += *((char *)header + i);
        return sum == 0;
}

int
is_acpi_supported(void)
{
        return acpi_supported;
}

void
acpi_init(void)
{
        struct rsdp_desc *rsdp;
        struct rsdt *rsdt;
        int sum;
        char *p;

        if ((rsdp = find_rsdp()) == NULL) {
                printk("ACPI not supported\r\n");
                return;
        }
        /*
         * Make sure the table is valid.
         * Casing the sum of all the struct's members to a byte
         * should give 0. See ACPI specification.
         */
        for (p = (char *)rsdp, sum = 0; p < (char *)rsdp +sizeof(*rsdp); p++)
                sum += *p;
        if ((uint8_t)sum) {
                printk("Invalid RSDP\r\n");
                return;
        }
        rsdt = (struct rsdt *)(virt_map_phys(rsdp->rsdt_addr & 0xFFFFF000) + (rsdp->rsdt_addr & VIRT_ADDR_FRAME_MASK));
        if (acpi_table_check(&rsdt->header) == 0) {
                printk("Invaid RSDT\r\n");
                return;
        }
        acpi_supported = 1;
}
