#ifndef _TTY_H
#define _TTY_H

#define TTY_BUF_SIZE	512

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
	struct tty_driver *t_driver;
	struct tty_queue t_writeq;
	struct tty_queue t_readq;
	int t_stopped;
};

struct tty *tty_driver_register(int n, struct tty_driver *driver);
int tty_write(int n, char *buf, int count);
int tty_init(void);

#endif /* _TTY_H */
