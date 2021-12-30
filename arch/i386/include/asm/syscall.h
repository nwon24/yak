#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>

/* Should be enough for now */
#define NR_SYSCALL 100

#define SYSCALL_IRQ	0x80

struct syscall_entry {
	uint32_t addr;
	uint32_t nr_args;
};

extern struct syscall_entry syscall_table[];

static inline void
register_syscall(int n, uint32_t addr, uint32_t nr_args)
{
	syscall_table[n].addr = addr;
	syscall_table[n].nr_args = nr_args;
}
 
#endif /* SYSCALL_H */
