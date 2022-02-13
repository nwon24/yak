/*
 * tty.c
 * TTY driver.
 */
#include <stddef.h>

#include <drivers/tty.h>

#include <fs/dev.h>

#include <generic/errno.h>

#include <kernel/debug.h>
#include <kernel/proc.h>

#define NR_TTY	6

#define TERMIOS_OFLAG(tp, flag)	((tp)->t_termios.c_oflag & (flag))
#define TERMIOS_LFLAG(tp, flag)	((tp)->t_termios.c_lflag & (flag))
#define TERMIOS_IFLAG(tp, flag)	((tp)->t_termios.c_iflag & (flag))
#define TERMIOS_CFLAG(tp, flag)	((tp)->t_termios.c_cflag & (flag))

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
tty_putch(int c, struct tty_queue *tq)
{
	if (!tty_queue_full(tq)) {
		*tq->tq_tail++ = (char)c;
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
tty_getch(struct tty_queue *tq)
{
	if (!tty_queue_empty(tq))
		return (int) *tq->tq_head++;
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
	tp->t_termios.c_cc[VEOF] = 'd' & 0x1F;
	tp->t_termios.c_cc[VEOL] = 0;
	tp->t_termios.c_cc[VERASE] = 0x7F;
	tp->t_termios.c_cc[VKILL] = 'u' & 0x1F;
	tp->t_termios.c_cc[VINTR] = 'c' & 0x1F;
	tp->t_termios.c_cc[VQUIT] = '|' & 0x1F;
	tp->t_termios.c_cc[VSTART] = 'q' & 0x1F;
	tp->t_termios.c_cc[VSTOP] = 's' & 0x1F;
	tp->t_termios.c_cc[VSUSP] = 'y' & 0x1F;
	tp->t_termios.c_cc[VTIME] = 0;
	tp->t_termios.c_cc[VMIN] = 1;
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
	/*
	 * Should not be here, which is why we panic.
	 */
	if (tp->t_open == 0 && n != DEBUG_TTY)
		panic("tty_write: TTY is not open!");
	i = count;
	p = buf;
	while (*p && i-- && !tty_queue_full(&tp->t_writeq))
		tty_putch(*p++, &tp->t_writeq);
	tty_flush(tp);
	tty_queue_reset(&tp->t_writeq);
	return p - buf;
}

int
tty_read(int n, char *buf, int count)
{
	struct tty *tp;
	char *p;
	int i;

	if ((tp = &tty_tab[n]) >= &tty_tab[NR_TTY])
		return -1;
	if (tp->t_open == 0 && n != DEBUG_TTY)
		panic("tty_read: TTY is not open!");
	i = count;
	p = buf;
	while (i-- && !tty_queue_empty(&tp->t_readq))
		*p++ = tty_getch(&tp->t_readq);
	return p - buf;
}

int
tty_rw(int minor, char *buf, int count, int rw)
{
	/*
	 * Not really needed.
	 */
	if (minor != 0)
		return -EINVAL;
	return (rw == WRITE) ? tty_write(current_process->tty, buf, count) : tty_read(current_process->tty, buf, count);
}

int
ttyx_rw(int minor, char *buf, int count, int rw)
{
	return (rw == WRITE) ? tty_write(minor, buf, count) : tty_read(minor, buf, count);
}

int
tty_open(int minor)
{
	struct tty *tp;

	if (minor < 0 || minor > NR_TTY)
		return -EINVAL;
	tp = tty_tab + minor;
	tp->t_open = 1;
	return minor;
}

int
tty_close(int minor)
{
	struct tty *tp;

	if (minor < 0 || minor > NR_TTY)
		return -EINVAL;
	tp = tty_tab + minor;
	tp->t_open = 0;
	return minor;
}

void
do_update_tty(char *buf)
{
	struct tty *tp;
	char *p;

	if (current_process->tty >= 0 && current_process->tty < NR_TTY) {
		tp = tty_tab + current_process->tty;
		if (!tp->t_open)
			return;
		for (p = buf; *p != '\0'; p++) {
			if (!tty_queue_full(&tp->t_readq))
				tty_putch(*p, &tp->t_readq);
			if (!tty_queue_full(&tp->t_writeq))
				tty_putch(*p, &tp->t_writeq);
		}
		tty_flush(tp);
		tty_queue_reset(&tp->t_writeq);
	}
}
