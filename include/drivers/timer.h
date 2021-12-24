#ifndef _TIMER_H
#define _TIMER_H

#include <stdint.h>

/* 100 times per second = 10ms */
#define HZ	100

struct timer_driver {
	int (*init)(void);
	void (*irq_handler)(void);
};

int timer_init(void);
struct timer_driver *register_timer_driver(struct timer_driver *driver);

extern uint32_t timer_ticks;
extern uint32_t starting_time;

#define CURRENT_TIME	(starting_time + timer_ticks / HZ)

#endif /* _TIMER_H */
