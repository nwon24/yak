/*
 * tty.c
 * TTY driver.
 * TODO: Implement the POSIX terminal interface as much as possible.
 */
#include <stddef.h>
#include <string.h>

#include <drivers/timer.h>
#include <drivers/tty.h>

#include <fs/dev.h>

#include <generic/errno.h>
#include <generic/termios.h>

#include <kernel/debug.h>
#include <kernel/proc.h>

#define NR_TTY	6

#define TERMIOS_OFLAG(tp, flag)	((tp)->t_termios.c_oflag & (flag))
#define TERMIOS_LFLAG(tp, flag)	((tp)->t_termios.c_lflag & (flag))
#define TERMIOS_IFLAG(tp, flag)	((tp)->t_termios.c_iflag & (flag))
#define TERMIOS_CFLAG(tp, flag)	((tp)->t_termios.c_cflag & (flag))

#define TERMIOS_VEOF(tp)	((tp)->t_termios.c_cc[VEOF])
#define TERMIOS_VEOL(tp)	((tp)->t_termios.c_cc[VEOL])
#define TERMIOS_VERASE(tp)	((tp)->t_termios.c_cc[VERASE])
#define TERMIOS_VERASE(tp)	((tp)->t_termios.c_cc[VERASE])
#define TERMIOS_VINTR(tp)	((tp)->t_termios.c_cc[VINTR])
#define TERMIOS_VKILL(tp)	((tp)->t_termios.c_cc[VKILL])
#define TERMIOS_VMIN(tp)	((tp)->t_termios.c_cc[VMIN])
#define TERMIOS_VTIME(tp)	((tp)->t_termios.c_cc[VTIME])
#define TERMIOS_VQUIT(tp)	((tp)->t_termios.c_cc[VQUIT])
#define TERMIOS_VSUSP(tp)	((tp)->t_termios.c_cc[VSUSP])
#define TERMIOS_VSTART(tp)	((tp)->t_termios.c_cc[VSTART])
#define TERMIOS_VSTOP(tp)	((tp)->t_termios.c_cc[VSTOP])

#define CANON		0
#define NONCANON	1

static void tty_process_input(struct tty *tp, int c);
static void tty_process_output(struct tty *tp, int c);
static int tty_read_case1(struct tty *tp, char *buf, int c, int vmin, int vtime);
static int tty_read_case2(struct tty *tp, char *buf, int c, int vmin);
static int tty_read_case3(struct tty *tp, char *buf, int c, int vtime);
static int tty_read_case4(struct tty *tp, char *buf, int c);
static int tty_canon_read(struct tty *tp, char *buf, int c);
static void tty_sleep(struct tty *tp, struct tty_queue *tq, int canon);
static void tty_set_timer(int vtime);
static void tty_clear_timer(void);

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
	memset(tq->tq_buf, 0, sizeof(tq->tq_buf));
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
	tp->t_termios.c_oflag = OPOST | ONLCR;
	tp->t_termios.c_lflag = ICANON | ECHO | ECHOK | ECHOE;
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
	while (*p && i-- && !tty_queue_full(&tp->t_writeq)) {
		int c;

		c = *p++;
		if (TERMIOS_OFLAG(tp, OPOST)) {
			if (c == '\n' && TERMIOS_OFLAG(tp, ONLCR))
				tty_putch('\r', &tp->t_writeq);
			else if (c == '\r' && TERMIOS_OFLAG(tp, OCRNL))
				c = '\n';
		}
		tty_putch(c, &tp->t_writeq);
	}
	tty_flush(tp);
	tty_queue_reset(&tp->t_writeq);
	return p - buf;
}

int
tty_read(int n, char *buf, int count)
{
	struct tty *tp;

	if ((tp = &tty_tab[n]) >= &tty_tab[NR_TTY])
		return -1;
	if (tp->t_open == 0 && n != DEBUG_TTY)
		panic("tty_read: TTY is not open!");
	if (TERMIOS_LFLAG(tp, ICANON)) {
		return tty_canon_read(tp, buf, count);
	} else {
		if (TERMIOS_VMIN(tp) > 0 && TERMIOS_VTIME(tp) > 0)
			return tty_read_case1(tp, buf, count, TERMIOS_VMIN(tp), TERMIOS_VTIME(tp));
		if (TERMIOS_VMIN(tp) > 0 && TERMIOS_VTIME(tp) == 0)
			return tty_read_case2(tp, buf, count, TERMIOS_VMIN(tp));
		if (TERMIOS_VMIN(tp) == 0 && TERMIOS_VTIME(tp) > 0)
			return tty_read_case3(tp, buf, count, TERMIOS_VTIME(tp));
		return tty_read_case4(tp, buf, count);
	}
	return 0;
}

/*
 * tty_canon_read: Canonical read.
 */
static int
tty_canon_read(struct tty *tp, char *buf, int c)
{
	char *p;

	if (!tty_queue_empty(&tp->t_readq) && *tp->t_readq.tq_tail == TERMIOS_VEOF(tp))
		return 0;
	tty_sleep(tp, &tp->t_readq, CANON);
	if (current_process->sigpending)
		return -EINTR;
	p = buf;
	while (!tty_queue_empty(&tp->t_readq) && p - buf < c)
		*p++ = tty_getch(&tp->t_readq);
	return p - buf;
}

static void
tty_sleep(struct tty *tp, struct tty_queue *tq, int canon)
{
	int block;

	block = (current_process->tty_block != NO_TTY_BLOCK);
	if (canon == NONCANON) {
		while (tty_queue_empty(tq)) {
			sleep(tq, PROC_SLEEP_INTERRUPTIBLE);
			if (block && current_process->tty_block == NO_TTY_BLOCK)
				return;
		}
	} else {
		while (*tq->tq_tail != '\n' &&
		       *tq->tq_tail != TERMIOS_VEOF(tp) &&
		       *tq->tq_tail != TERMIOS_VEOL(tp))
			sleep(tq, PROC_SLEEP_INTERRUPTIBLE);
	}
}

static void
tty_set_timer(int vtime)
{
	current_process->tty_block = timer_ticks + vtime * (HZ / 10);
}

static void tty_clear_timer(void)
{
	current_process->tty_block = NO_TTY_BLOCK;
}

/*
 * tty_read_case1: non-canonical read when
 * MIN>0, TIME>0.
 */
static int
tty_read_case1(struct tty *tp, char *buf, int c, int vmin, int vtime)
{
	int req;
	char *p;

	p = buf;
	req = (c < vmin) ? c : vmin;
	if (tty_queue_empty(&tp->t_readq))
		tty_sleep(tp, &tp->t_readq, NONCANON);
 loop:
	while (!tty_queue_empty(&tp->t_readq)) {
		*p++ = tty_getch(&tp->t_readq);
		if (p - buf == req)
			return req;
		tty_set_timer(vtime);
	}
	if (p - buf < req) {
		tty_sleep(tp, &tp->t_readq, NONCANON);
		if (current_process->sigpending)
			return -EINTR;
		if (current_process->tty_block == NO_TTY_BLOCK)
			/* VTIME expired */
			return p - buf;
	}
	goto loop;
}

/*
 * tty_read_case2: non-canonical read when
 * MIN>0 and TIME=0
 */
static int
tty_read_case2(struct tty *tp, char *buf, int c, int vmin)
{
	int req;
	char *p;

	req = (c < vmin) ? c : vmin;
	p = buf;
 loop:
	while (!tty_queue_empty(&tp->t_readq)) {
		*p++ = tty_getch(&tp->t_readq);
		if (p - buf == req)
			return req;
	}
	tty_sleep(tp, &tp->t_readq, NONCANON);
	if (current_process->sigpending)
		return -EINTR;
	goto loop;
}

/*
 * tty_read_case3: non-canonical read when MIN=0, TIME>0.
 */
static int
tty_read_case3(struct tty *tp, char *buf, int c, int vtime)
{
	(void) c;
	tty_set_timer(vtime);
	if (!tty_queue_empty(&tp->t_readq)) {
		*buf = tty_getch(&tp->t_readq);
		tty_clear_timer();
		return 1;
	}
	tty_sleep(tp, &tp->t_readq, NONCANON);
	if (current_process->sigpending)
		return -EINTR;
	return 0;
}

/*
 * tty_read_case4: non-canonical read where MIN=0 and TIME=0
 */
static int
tty_read_case4(struct tty *tp, char *buf, int c)
{
	char *p;

	(void) c;
	p = buf;
	while (!tty_queue_empty(&tp->t_readq))
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
do_update_tty(const char *buf)
{
	struct tty *tp;
	const char *p;
	int out;

	printk("tty lflag %x\r\n", tty_tab[0].t_termios.c_cflag);
	out = 0;
	if (current_process->tty >= 0 && current_process->tty < NR_TTY) {
		tp = tty_tab + current_process->tty;
		if (!tp->t_open)
			return;
		for (p = buf; *p != '\0'; p++) {
			tty_process_input(tp, *p);
			if (TERMIOS_LFLAG(tp, ECHO)) {
				tty_process_output(tp, *p);
				if (!out)
					out = 1;
			}
		}
		if (out) {
			tty_flush(tp);
			tty_queue_reset(&tp->t_writeq);
		}
	}
}

static void
tty_process_input(struct tty *tp, int c)
{
	if (tty_queue_full(&tp->t_readq)) {
		printk("WARNING: tty read input queue full\r\n");
		return;
	}
	if (c == '\r') {
		if (TERMIOS_IFLAG(tp, ICRNL))
			c = '\n';
		else if (TERMIOS_IFLAG(tp, IGNCR))
			return;
	}
	if (c == '\n' && TERMIOS_IFLAG(tp, INLCR))
		c = '\r';
	if (TERMIOS_LFLAG(tp, ECHOE) && c == TERMIOS_VERASE(tp)) {
		if (!tty_queue_empty(&tp->t_readq))
			tp->t_readq.tq_tail--;
	} else if (TERMIOS_LFLAG(tp, ECHOK) && c == TERMIOS_VKILL(tp)) {
		while (!tty_queue_empty(&tp->t_readq) && *tp->t_readq.tq_tail != '\r')
			tp->t_readq.tq_tail--;
	}
	tty_putch(c, &tp->t_readq);
}

static void
tty_process_output(struct tty *tp, int c)
{
	if (tty_queue_full(&tp->t_writeq)) {
		tty_flush(tp);
		tty_queue_reset(&tp->t_writeq);
	}

	if (TERMIOS_OFLAG(tp, OPOST)) {
		if (c == '\n' && TERMIOS_OFLAG(tp, ONLCR))
			tty_putch('\r', &tp->t_writeq);
		else if (c == '\r' && TERMIOS_OFLAG(tp, ONLRET))
			 return;
		else if (c == '\r' && TERMIOS_OFLAG(tp, OCRNL))
			c = '\n';
	}
	if (TERMIOS_LFLAG(tp, ECHOE) && c == TERMIOS_VERASE(tp)) {
		tty_flush(tp);
		tty_queue_reset(&tp->t_writeq);
		tty_putch('\b', &tp->t_writeq);
		tty_putch(' ', &tp->t_writeq);
		tty_putch('\b', &tp->t_writeq);
	} else if (TERMIOS_LFLAG(tp, ECHOK) && c == TERMIOS_VKILL(tp)) {
		const char *seq = "\033[1K";
		tty_flush(tp);
		tty_queue_reset(&tp->t_writeq);
		while (*seq != '\0')
			tty_putch(*seq++, &tp->t_writeq);
	} else {
		tty_putch(c, &tp->t_writeq);
	}
}
