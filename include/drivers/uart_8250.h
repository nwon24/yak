#ifndef UART_8250_H
#define UART_8250_H

enum {
	UART8250_WRITE,
	UART8250_READ
};

int uart8250_serial_init(void);

#endif /* UART_8250_H */
