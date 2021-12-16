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

struct rsdp_desc {
        char signature[8];
        uint8_t checksum;
        char oemid[6];
        uint8_t revision;
        uint32_t rsdt_addr;
}__attribute__((packed));

static struct rsdp_desc *find_rsdp(void);

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
void
acpi_init(void)
{
        struct rsdp_desc *rsdp;

        if ((rsdp = find_rsdp()) == NULL)
                printk("ACPI not supported.\r\n");
}
