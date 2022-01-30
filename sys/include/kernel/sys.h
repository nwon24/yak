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
int kernel_setpgid(pid_t pid, pid_t pgid);
int kernel_getpid(void);
int kernel_getppid(void);
int kernel_getpgid(pid_t pid);
int kernel_getpgrp(void);
int kernel_stime(time_t *tloc);

static inline void
sys_other_init(void)
{
	register_syscall(__NR_getuid, (size_t)kernel_getuid, 0);
	register_syscall(__NR_geteuid, (size_t)kernel_geteuid, 0);
	register_syscall(__NR_getgid, (size_t)kernel_getgid, 0);
	register_syscall(__NR_getegid, (size_t)kernel_getegid, 0);
	register_syscall(__NR_setuid, (size_t)kernel_setuid, 1);
	register_syscall(__NR_setgid, (size_t)kernel_setgid, 1);
	register_syscall(__NR_setsid, (size_t)kernel_setsid, 0);
	register_syscall(__NR_umask, (size_t)kernel_umask, 1);
	register_syscall(__NR_time, (size_t)kernel_time, 1);
	register_syscall(__NR_setpgid, (size_t)kernel_setpgid, 2);
	register_syscall(__NR_getpid, (size_t)kernel_getpid, 0);
	register_syscall(__NR_getppid, (size_t)kernel_getppid, 0);
	register_syscall(__NR_getpgid, (size_t)kernel_getpgid, 1);
	register_syscall(__NR_getpgrp, (size_t)kernel_getpgrp, 0);
	register_syscall(__NR_stime, (size_t)kernel_stime, 1);
}

#endif /* SYS_H */
