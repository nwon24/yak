/*
 * config.h
 * Contains all configurable options.
 */

#ifndef CONFIG_H
#define CONFIG_H

#define KERNEL_VIRT_BASE	0xC0000000

#define CONFIG_ARCH_X86

/* If it is ever needed */
#ifdef CONFIG_ARCH_X86
#define CONFIG_X86_ISA_I686
#endif /* CONFIG_ARCH_X86 */

#define CONFIG_DRIVER_8250_UART

#define CONFIG_DRIVER_FBDEV

/* Should be enough for now, right? */
#define CONFIG_DRIVER_NR_DRIVERS	100

/* No support for GPT yet. */
#define CONFIG_DISK_MBR

/* Compiled in font */
#define CONFIG_FONT_8X16_PSFU

#ifdef __GNUC__
/* A little bit of inline assembly doesn't hurt. As long as it is all in the arch/ directory. */
#define CONFIG_USE_INLINE_ASM
#endif /* __GNUC__ */

/* Optimised string functions are done using inline assembly. */
#ifdef CONFIG_USE_INLINE_ASM
#define CONFIG_USE_ARCH_STRING_H
#endif

#define CONFIG_FS_BUF_CACHE_SIZE	128
#define CONFIG_FS_BUF_HASH_SIZE		100

#endif /* CONFIG_H */
