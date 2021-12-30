#ifndef DEBUG_H
#define DEBUG_H

#include <drivers/tty.h>

#define DEBUG_TTY	SERIAL_TTY_START

void change_printk_tty(int tty);
int printk(const char *fmt, ...);
void panic(const char *msg);

#endif /* DEBUG_H */
