#ifndef _8250_UART_H
#define _8250_UART_H

enum {
	UART8250_WRITE,
	UART8250_READ
};

int uart8250_serial_init(void);

#endif /* _8260_UART_H */
