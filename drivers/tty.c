/*
 * tty.c
 * TTY driver.
 */
#include <stddef.h>

#include <drivers/tty.h>

#define NR_TTY	6

static struct tty tty_tab[NR_TTY];

#define TTY_NUM(tp)	((tp) - tty_tab >= SERIAL_TTY_START ? (tp) - tty_tab - SERIAL_TTY_START : (tp) - tty_tab)

static inline int
tty_queue_empty(struct tty_queue *tq)
{
	return tq->tq_head == tq->tq_tail;
}

static inline int
tty_queue_full(struct tty_queue *tq)
{
	return tq->tq_tail == tq->tq_buf + TTY_BUF_SIZE;
}

static inline int
tty_putch(int c, struct tty *tp)
{
	if (!tty_queue_full(&tp->t_writeq)) {
		*tp->t_writeq.tq_tail++ = (char)c;
		return c;
	}
	return 0;

}

static inline int
tty_flush(struct tty *tp)
{
	if (!tp->t_stopped)
		return tp->t_driver->driver_out(TTY_NUM(tp), &tp->t_writeq);
	return 0;
}

static inline int
tty_getch(struct tty *tp)
{
	if (!tty_queue_empty(&tp->t_readq))
		return (int) *tp->t_readq.tq_head++;
	return 0;
}

static inline void
tty_queue_reset(struct tty_queue *tq)
{
	tq->tq_head = tq->tq_tail = tq->tq_buf;
}

struct tty *
tty_driver_register(int n, struct tty_driver *driver)
{
	struct tty *tp;

	if (n >= NR_TTY)
		return NULL;
	tp = tty_tab + n;
	tp->t_driver = driver;
	return tp;
}

static struct tty *
tty_struct_init(int n)
{
	struct tty *tp;

	if (n >= NR_TTY)
		return NULL;
	tp = tty_tab + n;
	tty_queue_reset(&tp->t_writeq);
	tty_queue_reset(&tp->t_readq);
	tp->t_stopped = 0;
	return tp;
}

int
tty_init(void)
{
	int n;

	for (n = 0; n < NR_TTY; ++n)
		tty_struct_init(n);
	return 0;
}

int
tty_write(int n, char *buf, int count)
{
	struct tty *tp;
	char *p;
	int i;

	if ((tp = &tty_tab[n]) >= &tty_tab[NR_TTY])
		return -1;
	i = count;
	p = buf;
	while (*p && i--)
		tty_putch(*p++, tp);
	tty_flush(tp);
	tty_queue_reset(&tp->t_writeq);
	return p - buf;
}
