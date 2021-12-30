#ifndef PORT_IO_H
#define PORT_IO_H

#ifndef __ASSEMBLER__

#include <stdint.h>
#include <kernel/config.h>

#ifdef CONFIG_USE_INLINE_ASM
static inline void
outb(uint8_t data, uint16_t port)
{
	__asm__ volatile("outb %0, %1" : : "a" (data), "Nd" (port));
}

static inline uint8_t
inb(uint16_t port)
{
	uint8_t ret;

	__asm__ volatile("inb %1, %0" : "=a" (ret) : "Nd" (port));
	return ret;
}

static inline void
outw(uint16_t data, uint16_t port)
{
	__asm__ volatile("outw %0, %1" : : "a" (data), "Nd" (port));
}

static inline uint16_t
inw(uint16_t port)
{
	uint16_t ret;

	__asm__ volatile("inw %1, %0" : "=a" (ret) : "Nd" (port));
	return ret;
}

static inline void
outl(uint32_t data, uint32_t port)
{
	__asm__ volatile("outl %0, %1" : : "a" (data), "Nd" (port));
}

static inline uint32_t
inl(uint32_t port)
{
	uint32_t ret;

	__asm__ volatile("inl %1, %0" : "=a" (ret) : "Nd" (port));
	return ret;
}
#else /* CONFIG_USE_INLINE_ASM */

void port_outb(uint8_t data, uint16_t port);
uint8_t port_inb(uint16_t port);

void port_outw(uint16_t data, uint16_t port);
uint16_t port_inw(uint16_t port);

void port_outl(uint32_t data, uint32_t port);
uint32_t port_inl(uint32_t port);

#define outb	port_outb
#define inb	port_inb

#define outw	port_outw
#define inw	port_inw

#define outl	port_outl
#define inl	port_inl

#endif /* CONFIG_USE_INLINE_ASM */
#endif /* __ASSEMBLER__ */

#endif /* __ASSEMBLER__ */
