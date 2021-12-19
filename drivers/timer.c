/*
 * timer.c
 * Generic timer interface on top of device dependent timers.
 * Does virtually nothing now, but can be expanded to interface different timers.
 */

#include <drivers/timer.h>
#include <drivers/driver.h>

static struct timer_driver *timer_driver;

static void timer_driver_irq_handler(void);

static struct driver generic_timer_driver = {
	.irq = 1,
	.id = _DRIVERS_TIMER_DRIVER,
	.irq_handler = &timer_driver_irq_handler,
};

struct timer_driver *
register_timer_driver(struct timer_driver *driver)
{
	timer_driver = driver;
	return driver;
}

int
timer_init(void)
{
	register_driver(&generic_timer_driver);
	return 0;
}

static void
timer_driver_irq_handler(void)
{
	timer_driver->irq_handler();
}
