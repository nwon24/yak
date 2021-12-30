#ifndef IRQ_H
#define IRQ_H

void irq0_handler(void);
void irq3_handler(void);
void irq4_handler(void);

void driver_irq(int id);

#endif /* IRQ_H */
