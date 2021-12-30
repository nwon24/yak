#ifndef PIC_8259_H
#define PIC_8259_H

#define PIC1_BASE	0x20
#define PIC2_BASE	0xA0
#define PIC1_CMD	PIC1_BASE
#define PIC2_CMD	PIC2_BASE
#define PIC1_DATA	(PIC1_BASE + 1)
#define PIC2_DATA	(PIC2_BASE + 1)

#define PIC_EOI		0x20

#define PIC1_OFFSET	0x20
#define PIC2_OFFSET	0x28

#define IRQ_BASE	PIC1_OFFSET

#ifndef __ASSEMBLER__

#include <stdint.h>

uint8_t pic_set_mask(uint8_t line);
uint8_t pic_clear_mask(uint8_t line);
void pic_remap(void);
uint16_t pic_read_isr(void);
uint16_t pic_read_irr(void);

#endif /* __ASSEMBLER__ */

#endif /* PIC_8259_H */
