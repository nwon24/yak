#ifndef _DEBUG_H
#define _DEBUG_H

#define DEBUG_TTY	0

int printk(const char *fmt, ...);
void panic(const char *msg);

#endif /* _DEBUG_H */
