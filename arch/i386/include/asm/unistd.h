#ifndef ARCH_UNISTD_H
#define ARCH_UNISTD_H

#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2

#define __NR_exit	0x1
#define __NR_fork	0x2
#define __NR_read	0x3
#define __NR_write	0x4
#define __NR_open	0x5
#define __NR_close	0x6
#define __NR_waitpid	0x7
#define __NR_creat	0x8
#define __NR_link	0x9
#define __NR_unlink	0xA
#define __NR_chdir	0xC
#define __NR_time	0xD
#define __NR_mknod	0xE
#define __NR_chmod	0xF
#define __NR_lchown	0x10
#define __NR_lseek	0x13
#define __NR_getpid	0x14
#define __NR_setuid	0x17
#define __NR_getuid	0x18
#define __NR_alarm	0x1B
#define __NR_pause	0x1D
#define __NR_nice	0x22
#define __NR_sync	0x24
#define __NR_kill	0x25
#define __NR_rename	0x26
#define __NR_mkdir	0x27
#define __NR_rmdir	0x28
#define __NR_dup	0x29
#define __NR_setgid	0x2E
#define __NR_getgid	0x2F
#define __NR_signal	0x30
#define __NR_geteuid	0x31
#define __NR_getegid	0x32
#define __NR_fcntl	0x37
#define __NR_setpgid	0x38
#define __NR_umask	0x3C
#define __NR_chroot	0x3D
#define __NR_dup2	0x3F
#define __NR_getppid	0x40
#define __NR_getpgrp	0x41
#define __NR_setsid	0x42
#define __NR_symlink	0x53
#define __NR_fchmod	0x5E
#define __NR_getpgid	0x84
#define __NR_chown	0xB6

#endif /* ARCH_UNISTD_H */
