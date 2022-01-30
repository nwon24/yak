/*
 * init.c
 * Contains table of driver init functions.
 */

#include <stddef.h>

#include <kernel/config.h>

#include <drivers/drive.h>
#include <drivers/init.h>
#include <drivers/tty.h>
#include <drivers/fb.h>
#include <drivers/timer.h>
#include <drivers/virtual_console.h>

#ifdef CONFIG_DRIVER_8250_UART
#include <drivers/uart_8250.h>
#endif /* CONFIG_DRIVER_8250_UART */

#ifdef CONFIG_FS_EXT2
#include <fs/ext2.h>
#endif

#ifdef CONFIG_ARCH_X86
int pit_init(void);
#endif /* CONFIG_ARCH_X86 */

typedef int (*init_function)(void);

init_function critical_init_func_table[] = {
	tty_init,
#ifdef CONFIG_DRIVER_8250_UART
	uart8250_serial_init,
#endif
#ifdef CONFIG_ARCH_X86
	pit_init,
#endif /* CONFIG_ARCH_X86 */
	NULL
};

init_function init_func_table[] = {
#ifdef CONFIG_DRIVER_FBDEV
	fb_init,
#else
#error "Text mode not supported."
#endif /* CONFIG_DRIVER_FBDEV */
	vc_init,
	drive_init,
#ifdef CONFIG_FS_EXT2
	ext2_init,
#endif /* CONFIG_FS_EXT2 */
	NULL,
};

void
run_driver_init_functions(void)
{
	init_function *func;

	for (func = init_func_table; *func != NULL; func++)
		(*func)();
}

void
run_critical_driver_init_functions(void)
{
	init_function *func;

	for (func = critical_init_func_table; *func != NULL; func++)
		(*func)();
}
