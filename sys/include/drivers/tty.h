#ifndef TTY_H
#define TTY_H

#include <drivers/virtual_console.h>

#include <generic/termios.h>

#define TTY_BUF_SIZE	512

/* Serial ttys begin right after virtual consoles */
#define SERIAL_TTY_START	NR_VIRTUAL_CONSOLES

struct tty_queue {
	char tq_buf[TTY_BUF_SIZE];
	char *tq_head;
	char *tq_tail;
};

struct tty_driver {
	int (*driver_out)(int tty, struct tty_queue *tq);
	int driver_id;
};

struct tty {
	struct termios t_termios;
	struct tty_driver *t_driver;
	struct tty_queue t_writeq;
	struct tty_queue t_readq;
	int t_stopped;
	int t_open;
};

struct tty *tty_driver_register(int n, struct tty_driver *driver);
int tty_write(int n, char *buf, int count);
int tty_init(void);
int tty_open(int minor);
int tty_close(int minor);
int tty_rw(int minor, char *buf, int count, int rw);
int ttyx_rw(int minor, char *buf, int count, int rw);
void do_update_tty(char *buf);

#endif /* TTY_H */
