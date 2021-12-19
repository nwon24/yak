#ifndef _TIMER_H
#define _TIMER_H

/* 100 times per second = 10ms */
#define HZ	100

struct timer_driver {
	int (*init)(void);
	void (*irq_handler)(void);
};

int timer_init(void);
struct timer_driver *register_timer_driver(struct timer_driver *driver);

#endif /* _TIMER_H */
