#ifndef _INTERNAL_SYSCALL_H
#define _INTERNAL_SYSCALL_H

#include <hobbix/asm/syscall.h>

#ifndef __ASSEMBLER__
long syscall(int nr, ...);
#endif /* __ASSEMBLER__ */

#endif /* SYSCALL_H */
