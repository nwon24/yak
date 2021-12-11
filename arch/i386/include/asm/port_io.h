#ifndef _PORT_IO_H
#define _PORT_IO_H

#ifndef _ASSEMBLY_

#include <stdint.h>

/* static inline void
outb(uint8_t data, uint16_t port)
{
	__asm__ volatile("outb %0, %1" : : "a" (data), "Nd" (port));
}

static inline uint8_t
inb(uint16_t port)
{
	uint8_t ret;

	__asm__ volatile("inb %1, %0" : "=a" (ret) : "Nd" (port));
	return (ret);
} */

void port_outb(uint8_t data, uint16_t port);
uint8_t port_inb(uint16_t port);

#define outb	port_outb
#define inb	port_inb

#endif /* _ASSEMBLY_ */

#endif /* _ASSEMBLY_ */
