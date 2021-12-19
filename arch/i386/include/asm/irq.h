#ifndef _IRQ_H
#define _IRQ_H

void irq0_handler(void);
void irq3_handler(void);
void irq4_handler(void);

void driver_irq(int id);

#endif /* _IRQ_H */
