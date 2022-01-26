#ifndef SYS_H
#define SYS_H

#include <asm/syscall.h>
#include <asm/types.h>

#include <generic/unistd.h>

/*
 * Misc. syscalls from kernel/sys.c go here.
 */

uid_t kernel_getuid(void);
gid_t kernel_getgid(void);
uid_t kernel_geteuid(void);
gid_t kernel_getegid(void);
int kernel_setuid(uid_t uid);
int kernel_setgid(gid_t gid);
pid_t kernel_setsid(void);
mode_t kernel_umask(mode_t cmask);
time_t kernel_time(time_t *tloc);

static inline void
sys_other_init(void)
{
	register_syscall(__NR_getuid, (uint32_t)kernel_getuid, 0);
	register_syscall(__NR_geteuid, (uint32_t)kernel_geteuid, 0);
	register_syscall(__NR_getgid, (uint32_t)kernel_getgid, 0);
	register_syscall(__NR_getegid, (uint32_t)kernel_getegid, 0);
	register_syscall(__NR_setuid, (uint32_t)kernel_setuid, 1);
	register_syscall(__NR_setgid, (uint32_t)kernel_setgid, 1);
	register_syscall(__NR_setsid, (uint32_t)kernel_setsid, 0);
	register_syscall(__NR_umask, (uint32_t)kernel_umask, 1);
	register_syscall(__NR_time, (uint32_t)kernel_time, 1);
}

#endif /* SYS_H */
