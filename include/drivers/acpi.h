#ifndef ACPI_H_
#define ACPI_H_

#include <stdint.h>

enum acpi_version {
	ACPI_1_0,
	ACPI_2_0,
};

void acpi_init(void);
int is_acpi_supported(void);
uint16_t get_acpi_boot_arch_flags(void);

#endif /* ACPI_H_ */
