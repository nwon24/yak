#ifndef SYSCALL_H
#define SYSCALL_H

#define SYSCALL_IRQ	0x80

#ifndef __LIBRARY__

#ifndef __ASSEMBLER__

#include <stdint.h>
#include <stddef.h>

/* Should be enough for now */
#define NR_SYSCALL 100

struct syscall_entry {
	size_t addr;
	uint32_t nr_args;
};

extern struct syscall_entry syscall_table[];

static inline void
register_syscall(int n, size_t addr, uint32_t nr_args)
{
	syscall_table[n].addr = addr;
	syscall_table[n].nr_args = nr_args;
}

#endif /* __ASSEMBLER__ */

#endif /* __LIBRARY__ */

#endif /* SYSCALL_H */
