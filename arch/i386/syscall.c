/*
 * syscall.c
 * Defines the system call table.
 */
#include <asm/syscall.h>

struct syscall_entry syscall_table[NR_SYSCALL];
