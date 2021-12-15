/*
 * config.h
 * Contains all configurable options.
 */

#ifndef _CONFIG_H
#define _CONFIG_H

#define KERNEL_VIRT_BASE	0xC0000000

#define _CONFIG_ARCH_X86

/* If it is ever needed */
#ifdef _CONFIG_ARCH_X86
#define _CONFIG_X86_ISA_I686
#endif /* _CONFIG_ARCH_X86 */

#define _CONFIG_DRIVER_8250_UART

#define _CONFIG_DRIVER_FBDEV

/* Should be enough for now, right? */
#define _CONFIG_DRIVER_NR_DRIVERS	100

/* Compiled in font */
#define _CONFIG_FONT_8X16_PSFU

#ifdef __GNUC__
/* A little bit of inline assembly doesn't hurt. As long as it is all in the arch/ directory. */
#define _CONFIG_USE_INLINE_ASM
#endif /* __GNUC__ */

/* Optimised string functions are done using inline assembly. */
#ifdef _CONFIG_USE_INLINE_ASM
#define _CONFIG_USE_ARCH_STRING_H
#endif

#endif /* _CONFIG_H */
