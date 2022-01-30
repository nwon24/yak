/*
 * 8250_uart.c
 * Contains a driver for the 8250 UART serial device.
 */

#include <stddef.h>

#include <generic/string.h>

#include <kernel/config.h>

#ifdef CONFIG_ARCH_X86

#include <kernel/debug.h>

#include <asm/pic_8259.h>
#include <asm/port_io.h>
#include <asm/idt.h>
#include <asm/irq.h>
#include <asm/segment.h>

#include <drivers/uart_8250.h>
#include <drivers/driver.h>
#include <drivers/tty.h>

#define COM1_PORT_BASE	0x3F8
#define COM2_PORT_BASE	0x2F8
#define COM3_PORT_BASE	0x3E8
#define COM4_PORT_BASE	0x2E8

#define COM_1_3_IRQ	(IRQ_BASE + 4)
#define COM_2_4_IRQ	(IRQ_BASE + 3)

#define DATA_REG	0
#define INT_ENABLE	1
#define DIVISOR_LOW	0
#define DIVISOR_HIGH	1
#define FCR		2
#define INTERRUPT_ID	2
#define LINE_CTRL	3
#define MODEM_CTRL	4
#define LINE_STATUS	5
#define MODEM_STATUS	6
#define SCRATCH		7

#define DLAB		(1 << 7)
#define NR_BITS_8	3
#define NR_BITS_7	2
#define NR_BITS_6	1
#define NR_BITS_5	0

/* Bits of the Interrupt Indentification Register. */
#define IIR_INTERRUPT_PENDING	1
/* These bits begin at bit 1 */
#define IIR_LINE_STATUS_INT	3
#define IIR_DATA_AVAILABLE_INT	2
#define IIR_TRANSMITTER_EMPTY_INT	1
#define IIR_MODEM_STATUS_INT	0

/*
 * Bits of FCR (FIFO Control Register).
 */
#define FIFO_14_BYTE_TRIGGER	(3 << 6)
#define FIFO_ENABLE	1
#define FIFO_CLEAR	(3 << 1)

/*
 * Bits of Modem Control Register (offset 4)
 * AUX_OUTPUT_2 must be set for interrupts to be enabled.
 */
#define AUX_OUTPUT_1	(1 << 2)
#define AUX_OUTPUT_2	(1 << 3)
#define RTS		(1 << 1)
#define DTR		1
/* Used to test chip */
#define LOOPBACK_MODE	(1 << 4)

#define TEST_BYTE	0xAE

#define IER_MODEM_STATUS_INT		(1 << 3)
#define IER_RECEIVER_LINE_STATUS_INT	(1 << 2)
#define IER_TRANSMITTER_EMPTY_INT		(1 << 1)
#define IER_RECEIVED_DATA_INT		1

#define COM_PORT(base, reg)	((base) + (reg))

#define DLAB_DIVISOR	3

#define NR_UART8250_REQUESTS	5

enum {
	PORT_INIT_SUCCESS,
	PORT_INIT_FAIL
};

struct uart8250_request {
	char *buf;
	int irq;
	int free;
	int rw;
	uint16_t port;
	struct uart8250_request *next;
};

static struct uart8250_request request_tab[NR_UART8250_REQUESTS];
static struct uart8250_request *current_req = NULL;

static int uart8250_tty_out(int tty, struct tty_queue *tq);
static struct tty_driver uart8250_tty_driver = {
	.driver_out = uart8250_tty_out,
	.driver_id = DRIVERS_UART8250_DRIVER
};

static void handle_com_irq(void);
static struct driver uart8250_driver = {
	.irq = 1,
	.id = DRIVERS_UART8250_DRIVER,
	.irq_handler = &handle_com_irq,
};

static void uart8250_start_req(struct uart8250_request *req);
static int uart8250_port_init(uint16_t port);
static struct uart8250_request *uart8250_find_free_req(void);
static int uart8250_rw(int port, char *buf, int rw, int count);
static void uart8250_start_req(struct uart8250_request *req);
static void uart8250_add_req(struct uart8250_request *req);

/*
 * Initialise one of the COM ports.
 * Return either PORT_INIT_SUCCESS or PORT_INIT_FAIL
 */
static int
uart8250_port_init(uint16_t port)
{
	outb(0, COM_PORT(port, INT_ENABLE));
	outb(DLAB, COM_PORT(port, LINE_CTRL));
	outb(DLAB_DIVISOR & 0xFF, COM_PORT(port, DIVISOR_LOW));
	outb((DLAB_DIVISOR >> 8) & 0xFF, COM_PORT(port, DIVISOR_HIGH));
	outb(NR_BITS_8, COM_PORT(port, LINE_CTRL));
	outb(FIFO_14_BYTE_TRIGGER | FIFO_ENABLE | FIFO_CLEAR, COM_PORT(port, FCR));
	outb(RTS | DTR | AUX_OUTPUT_2, COM_PORT(port, MODEM_CTRL));
	outb(LOOPBACK_MODE, COM_PORT(port, MODEM_CTRL));
	/*
	 * Test if hardware is faulty.
	 * Do this by sending a byte to the data register and reading it again to see
	 * if it is the same.
	 */
	outb(TEST_BYTE, COM_PORT(port, DATA_REG));
	if (inb(COM_PORT(port, DATA_REG)) != TEST_BYTE)
		return PORT_INIT_FAIL;

	/*
	 * Hardware is not faulty.
	 */
	outb(AUX_OUTPUT_1 | AUX_OUTPUT_2 | RTS | DTR, COM_PORT(port, MODEM_CTRL));
	return PORT_INIT_SUCCESS;
}

/*
 * Try to initialise each of the COM ports, setting the interrupt handlers as well.
 * If at least one of the ports works, return 0.
 * Otherwise, return -1 to indicate no serial hardware available.
 */
int
uart8250_serial_init(void)
{
	struct uart8250_request *req;

	if (uart8250_port_init(COM1_PORT_BASE) == PORT_INIT_SUCCESS
	    || uart8250_port_init(COM3_PORT_BASE) == PORT_INIT_SUCCESS) {
		set_idt_entry(COM_1_3_IRQ, (uint32_t)irq4_handler, KERNEL_CS_SELECTOR, DPL_0, IDT_32BIT_INT_GATE);
		pic_clear_mask(COM_1_3_IRQ);
	} else if (uart8250_port_init(COM2_PORT_BASE) == PORT_INIT_SUCCESS
		   || uart8250_port_init(COM4_PORT_BASE) == PORT_INIT_SUCCESS) {
		set_idt_entry(COM_2_4_IRQ, (uint32_t)irq3_handler, KERNEL_CS_SELECTOR, DPL_0, IDT_32BIT_INT_GATE);
		pic_clear_mask(COM_2_4_IRQ);
	}

	for (req = request_tab; req < &request_tab[NR_UART8250_REQUESTS]; req++) {
		if (req < &request_tab[NR_UART8250_REQUESTS - 1])
			req->next = req + 1;
		req->buf = NULL;
		req->irq = 0;
		req->free = 1;
	}
	req->next = request_tab;
	register_driver(&uart8250_driver);
	if (tty_driver_register(DEBUG_TTY, &uart8250_tty_driver) == NULL)
		return -1;
	return 0;
}

static struct uart8250_request *
uart8250_find_free_req(void)
{
	struct uart8250_request *req;

	for (req = request_tab; req < &request_tab[NR_UART8250_REQUESTS]; req++) {
		if (req->free)
			return req;
	}
	return NULL;
}

static int
uart8250_rw(int port, char *buf, int rw, int count)
{
	struct uart8250_request *req;
	int ports[] = { COM1_PORT_BASE, COM2_PORT_BASE, COM3_PORT_BASE, COM4_PORT_BASE };

	if ((req = uart8250_find_free_req()) == NULL)
		/* Should really be a sleep here. */
		return -1;
	req->buf = buf;
	req->irq = count;
	req->port = ports[port];
	req->next = NULL;
	req->free = 0;
	req->rw = rw;
	if (current_req == NULL)
		uart8250_start_req(req);
	else
		uart8250_add_req(req);
	return 0;
}

static void
uart8250_start_req(struct uart8250_request *req)
{
	current_req = req;
	if (req->rw == UART8250_WRITE)
		outb(inb(COM_PORT(req->port, INT_ENABLE)) | IER_TRANSMITTER_EMPTY_INT, COM_PORT(req->port, INT_ENABLE));
	else if (req->rw == UART8250_READ)
		outb(inb(COM_PORT(req->port, INT_ENABLE)) | IER_RECEIVED_DATA_INT, COM_PORT(req->port, INT_ENABLE));
}

static void
uart8250_add_req(struct uart8250_request *req)
{
	struct uart8250_request *p;

	for (p = current_req; p->next != NULL; p = p->next);
	p->next = req;
}

static void
handle_com_irq(void)
{
	uint16_t iir;

	if (!current_req)
		/* This shouldn't happen. */
		return;
	iir = inb(COM_PORT(current_req->port, INTERRUPT_ID));
	/*
	 * For some reason, if bit 0 is set, it means the interrupt has
	 * already been serviced. A bit counter intuitive.
	 */
	if ((iir & IIR_INTERRUPT_PENDING))
		return;
	iir >>= 1;
	iir &= 7;
	if (iir == IIR_LINE_STATUS_INT)
		inb(COM_PORT(current_req->port, LINE_STATUS));
	else if (iir == IIR_DATA_AVAILABLE_INT)
		*current_req->buf++ = inb(COM_PORT(current_req->port, DATA_REG));
	else if (iir == IIR_MODEM_STATUS_INT)
		inb(COM_PORT(current_req->port, MODEM_STATUS));
	else if (iir == IIR_TRANSMITTER_EMPTY_INT)
		outb(*current_req->buf++, COM_PORT(current_req->port, DATA_REG));
	current_req->irq--;
	if (!current_req->irq) {
		if (current_req->rw == UART8250_WRITE) {
			uint8_t i;
			/*
			 * Disable transmitter empty interrupt so that the UART doesn't
			 * send an interrupt waiting for the next character when we have
			 * finsished sending.
			 */
			i = inb(COM_PORT(current_req->port, INT_ENABLE));
			i &= (~IER_TRANSMITTER_EMPTY_INT) & 0xF;
			outb(i, COM_PORT(current_req->port, INT_ENABLE));
		}
		current_req->free = 1;
		if (current_req->next)
			uart8250_start_req(current_req->next);
		else
			current_req = NULL;
	}
}

static int uart8250_tty_out(int tty, struct tty_queue *tq)
{
	int nr;

	nr = tq->tq_tail - tq->tq_head;
	if (nr < 0)
		return -1;
	uart8250_rw(tty, tq->tq_head, UART8250_WRITE, nr);
	return nr;
}

#endif /* CONFIG_ARCH_X86 */
