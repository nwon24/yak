#ifndef DRIVER_H
#define DRIVER_H

#include <kernel/config.h>

/*
 * Numbers aren't important here.
 */
#define	DRIVERS_TTY_DRIVER		0
#ifdef CONFIG_DRIVER_8250_UART	
#define	DRIVERS_UART8250_DRIVER	1
#endif /* CONFIG_DRIVER_8250_UART */
#define DRIVERS_VIRTUAL_CONSOLE_DRIVER	2
#define DRIVERS_TIMER_DRIVER		3
#ifdef CONFIG_ARCH_X86
#define DRIVERS_PIT_DRIVER	3
#else
#error "No other timer driver supported yet."
#endif /* DRIVERS_PIT_DRIVER */
#define DRIVERS_DISK_DRIVER	4

#ifndef __ASSEMBLER__
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
#endif /* __ASSEMBLER__ */

#endif /* DRIVER_H */
