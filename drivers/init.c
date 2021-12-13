/*
 * init.c
 * Contains table of driver init functions.
 */

#include <stddef.h>

#include <kernel/config.h>

#include <drivers/init.h>
#include <drivers/tty.h>

#ifdef _CONFIG_DRIVER_8250_UART
#include <drivers/8250_uart.h>
#endif /* _CONFIG_DRIVER_8250_UART */

init_function init_func_table[] = {
#ifdef _CONFIG_DRIVER_8250_UART
	uart8250_serial_init,
#endif /* _CONFIG_DRIVER_8250_UART */
	tty_init,
	NULL,
};

void run_driver_init_functions(void)
{
	init_function *func;

	for (func = init_func_table; *func != NULL; func++)
		(*func)();
}
