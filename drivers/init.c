/*
 * init.c
 * Contains table of driver init functions.
 */

#include <stddef.h>

#include <kernel/config.h>

#include <drivers/init.h>
#include <drivers/tty.h>
#include <drivers/fb.h>
#include <drivers/timer.h>
#include <drivers/virtual_console.h>

#ifdef _CONFIG_DRIVER_8250_UART
#include <drivers/8250_uart.h>
#endif /* _CONFIG_DRIVER_8250_UART */

#ifdef _CONFIG_ARCH_X86
int pit_init(void);
#endif /* _CONFIG_ARCH_X86 */

typedef int (*init_function)(void);

init_function critical_init_func_table[] = {
	tty_init,
#ifdef _CONFIG_DRIVER_8250_UART
	uart8250_serial_init,
#endif
#ifdef _CONFIG_ARCH_X86
	pit_init,
#endif /* _CONFIG_ARCH_X86 */
	NULL
};

init_function init_func_table[] = {
#ifdef _CONFIG_DRIVER_FBDEV
	fb_init,
#endif /* _CONFIG_DRIVER_FBDEV */
	vc_init,
	timer_init,
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
