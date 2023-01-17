/*
 * driver_irq.c
 * A simple file that calls the appropriate driver interrupt handler.
 */

#include <kernel/config.h>
#include <kernel/debug.h>

#include <drivers/driver.h>

void
driver_irq(int id)
{
	struct driver **dp;
	
	for (dp = driver_tab; dp < &driver_tab[CONFIG_DRIVER_NR_DRIVERS]; dp++) {
		if (!*dp)
			continue;
		if ((*dp)->irq && (*dp)->id == id) {
			(*dp)->irq_handler();
			return;
		}
	}
}
