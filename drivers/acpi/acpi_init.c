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

#define FADT_SIGNATURE	"FACP"

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

struct generic_address_structure {
	uint8_t address_space;
	uint8_t bit_width;
	uint8_t bit_offset;
	uint8_t access_size;
	uint64_t address;
};

struct fadt {
	struct acpi_sdt_header header;

	uint32_t firmware_ctrl;
	uint32_t dsdt;

	uint8_t reserved;

	uint8_t preferred_power_management_profile;
	uint16_t sci_interrupt;
	uint32_t smi_command_port;
	uint8_t acpi_enable;
	uint8_t acpi_disable;
	uint8_t s4bios_req;
	uint8_t pstate_control;
	uint32_t pm1a_event_block;
	uint32_t pm1b_event_block;
	uint32_t pm1a_control_block;
	uint32_t pm1b_control_block;
	uint32_t pm2_control_block;
	uint32_t pm_timer_block;
	uint32_t gpe0_block;
	uint32_t gpe1_block;
	uint8_t pm1_event_length;
	uint8_t pm1_control_length;
	uint8_t pm2_control_length;
	uint8_t pmt_timer_length;
	uint8_t gpe0_length;
	uint8_t gpe1_length;
	uint8_t gpe1_base;
	uint8_t cstate_control;
	uint16_t worst_c2_latency;
	uint16_t worst_c3_latency;
	uint16_t flush_size;
	uint16_t flush_stride;
	uint8_t duty_offset;
	uint8_t duty_width;
	uint8_t day_alarm;
	uint8_t month_alarm;
	uint8_t century;

	uint16_t boot_architecture_flags;

	uint8_t reserved2;
	uint32_t flags;

	struct generic_address_structure reset_reg;

	uint8_t reset_value;
	uint8_t reserved3[3];

	/* 64 bit pointers - not able to be used on 32 bit systems */
	uint64_t x_firmware_control;
	uint64_t x_dsdt;

	struct generic_address_structure x_pm1a_event_block;
	struct generic_address_structure x_pm1b_event_block;
	struct generic_address_structure x_pm1a_control_block;
	struct generic_address_structure x_pm1b_control_block;
	struct generic_address_structure x_pm2_control_block;
	struct generic_address_structure x_pmt_timer_block;
	struct generic_address_structure x_gpe0_block;
	struct generic_address_structure x_gpe1_block;
}__attribute__((packed));

static struct fadt fadt;

static struct rsdp_desc *find_rsdp(void);
static int acpi_table_check(struct acpi_sdt_header *header);
static void *acpi_find_table(uint32_t phys, char *sig);

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

static void *
acpi_find_table(uint32_t phys, char *sig)
{
	uint32_t virt;
	struct acpi_sdt_header *header;

	virt = virt_map_phys(phys & 0xFFFFF000);
	header = (struct acpi_sdt_header *)(virt + (phys & 0xFFF));
	if (memcmp(header->signature, sig, 4) == 0)
		return (void *)header;
	virt_unmap_virt(virt);
	return NULL;
}

int
is_acpi_supported(void)
{
        return acpi_supported;
}

uint16_t
get_acpi_boot_arch_flags(void)
{
	return fadt.boot_architecture_flags;
}

void
acpi_init(void)
{
        struct rsdp_desc *rsdp;
        struct acpi_sdt_header *rsdt;
	struct fadt *fadt_ptr = NULL;
	uint32_t *ptrs;
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
        rsdt = (struct acpi_sdt_header *)(virt_map_phys(rsdp->rsdt_addr & 0xFFFFF000) + (rsdp->rsdt_addr & VIRT_ADDR_FRAME_MASK));
        if (acpi_table_check(rsdt) == 0) {
                printk("Invaid RSDT\r\n");
                return;
        }
	ptrs = (uint32_t *)((char *)rsdt + sizeof(*rsdt));
	while (ptrs < (uint32_t *)((char *)rsdt + rsdt->length)) {
		if ((fadt_ptr = acpi_find_table(*ptrs, FADT_SIGNATURE)))
			break;
		ptrs++;
	}
	if (fadt_ptr == NULL || (acpi_table_check(&fadt_ptr->header) == 0)) {
		printk("fadt_ptr %p\r\n", fadt_ptr);
		printk("Unable to find FADT. Not there or invalid checksum.\r\n");
		return;
	}
	fadt = *fadt_ptr;
	virt_unmap_virt((uint32_t)rsdt & 0xFFFFF000);
	virt_unmap_virt((uint32_t)fadt_ptr & 0xFFFFF000);
        acpi_supported = 1;
}
