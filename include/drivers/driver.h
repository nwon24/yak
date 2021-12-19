#ifndef _DRIVER_H
#define _DRIVER_H

#include <kernel/config.h>

/*
 * Numbers aren't important here.
 */
#define	_DRIVERS_TTY_DRIVER		0
#ifdef _CONFIG_DRIVER_8250_UART	
#define	_DRIVERS_UART8250_DRIVER	1
#endif /* _CONFIG_DRIVER_8250_UART */
#define _DRIVERS_VIRTUAL_CONSOLE_DRIVER	2
#define _DRIVERS_TIMER_DRIVER		3
#ifdef _CONFIG_ARCH_X86
#define _DRIVERS_PIT_DRIVER	3
#else
#error "No other timer driver supported yet."
#endif /* _DRIVERS_PIT_DRIVER */

#ifndef _ASSEMBLY_
struct driver {
	int irq;	/* Handles IRQs? */
	void (*irq_handler)(void);	/* The actual handler. */
	int id;	/* Driver ID. */
};

extern struct driver *driver_tab[];

static inline void register_driver(struct driver *d)
{
	driver_tab[d->id] = d;
}
#endif /* _ASSEMBLY_ */

#endif /* _DRIVER_H */
