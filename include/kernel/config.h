/*
 * config.h
 * Contains all configurable options.
 */

#ifndef _CONFIG_H
#define _CONFIG_H

#define KERNEL_VIRT_BASE	0xC0000000

#define _CONFIG_ARCH_X86

#define _CONFIG_DRIVER_8250_UART

/* Should be enough for now, right? */
#define _CONFIG_DRIVER_NR_DRIVERS	100

#endif /* _CONFIG_H */
