/*
 * printk.c
 * An internal version of printf() for the kernel.
 * Used for debugging. Much simpler than regular printf.
 */

#include <stdarg.h>

#include <drivers/tty.h>

#include <kernel/debug.h>

/*
 * Need this defined somewhere in the architecture dependent files.
 */
void cpu_stop(void);

static int printk_tty = DEBUG_TTY;

enum signedness {
	SIGNED,
	UNSIGNED,
};

static int vsprintf(char *buf, const char *fmt, va_list args);
static char *itoa(char *s, int n, int base, enum signedness);
static char *reverse_str(char *s, int n);

int
printk(const char *fmt, ...)
{
	char buf[1024];
	va_list args;
	int nr;

	va_start(args, fmt);
	nr = vsprintf(buf, fmt, args);
	va_end(args);
	return tty_write(printk_tty, buf, nr);
}

void
change_printk_tty(int tty)
{
	printk_tty = tty;
}

void
panic(const char *msg)
{
	printk("Kernel panic: %s\r\n", msg);
	cpu_stop();
}

static char *
reverse_str(char *s, int n)
{
	int tmp, i;

	for (i = 0; i < n >> 1; i++) {
		tmp = s[i];
		s[i] = s[n - i - 1];
		s[n - i - 1] = tmp;
	}
	return s;
}

static char *
itoa(char *s, int n, int base, enum signedness signedness)
{
	int ns;
	unsigned int nu;
	char *p = s;
	const char *digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	if (signedness == SIGNED) {
		ns = n;
		if (ns < 0 && base == 10) {
			ns = -ns;
			*p++ = '-';
		}
		do
			*p++ = digits[ns % base];
		while ((ns /= base));
	} else {
		nu = (unsigned int)n;
		do
			*p++ = digits[nu % base];
		while ((nu /= base));
	}

	reverse_str(s, p - s);
	return p;
}

static int
vsprintf(char *buf, const char *fmt, va_list args)
{
	char *p = buf, *tmp;
	int n;

	while (*fmt) {
		if (*fmt != '%') {
			*p++ = *fmt++;
			continue;
		}
		switch (*++fmt) {
		case 'i':
		case 'd':
			n = va_arg(args, int);
			p = itoa(p, n, 10, SIGNED);
			break;
		case 'o':
			n = va_arg(args, int);
			p = itoa(p, n, 8, SIGNED);
			break;
		case 'u':
			n = va_arg(args, unsigned int);
			p = itoa(p, n, 10, UNSIGNED);
			break;
		case 'x':
			n = va_arg(args, unsigned int);
			p = itoa(p, n, 16, UNSIGNED);
			break;
		case 'p':
			n = (int)va_arg(args, void *);
			p = itoa(p, n, 16, UNSIGNED);
			break;
		case 'c':
			*p++ = (char)va_arg(args, int);
			break;
		case 's':
			tmp = va_arg(args, char *);
			while (*tmp)
				*p++ = *tmp++;
			break;
		default:
			*p++ = '%';
			*p++ = *fmt;
			break;
		}
		fmt++;
	}
	*p = '\0';
	return p - buf;
}

