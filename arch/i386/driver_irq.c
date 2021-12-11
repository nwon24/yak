/*
 * driver_irq.c
 * A simple file that calls the appropriate driver interrupt handler.
 */

#include <kernel/config.h>

#include <drivers/driver.h>

void
driver_irq(int id)
{
	struct driver **dp;

	for (dp = driver_tab; dp < &driver_tab[_CONFIG_DRIVER_NR_DRIVERS]; dp++) {
		if (!*dp)
			continue;
		if ((*dp)->irq) {
			if ((*dp)->id == id)
				(*dp)->irq_handler();
		}
	}
}
