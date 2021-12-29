/*
 * timer.c
 * Generic timer interface on top of device dependent timers.
 * Does virtually nothing now, but can be expanded to interface different timers.
 */

#include <drivers/timer.h>
#include <drivers/driver.h>

#include <kernel/debug.h>
#include <kernel/proc.h>

uint32_t get_current_time(void);

uint32_t startup_time;
uint32_t timer_ticks;

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
	/* Not exactly related to the timer, but close enough. */
	startup_time = get_current_time();
	register_driver(&generic_timer_driver);
	timer_driver->init();
	return 0;
}

static void
timer_driver_irq_handler(void)
{
	timer_ticks++;
	timer_driver->irq_handler();
	if (current_process != FIRST_PROC && current_process->priority > 1) {
		current_process->priority--;
		adjust_proc_queues(current_process);
	}
	if (!current_process->counter--) {
		current_process->counter = 0;
		current_process->state = PROC_RUNNABLE;
	}
	schedule();
}
