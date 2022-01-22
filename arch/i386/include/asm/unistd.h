#ifndef ARCH_UNISTD_H
#define ARCH_UNISTD_H

#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2

#define __NR_exit	0x0
#define __NR_fork	0x2
#define __NR_read	0x3
#define __NR_write	0x4
#define __NR_open	0x5
#define __NR_close	0x6
#define __NR_creat	0x8
#define __NR_link	0x9
#define __NR_unlink	0xA
#define __NR_chdir	0xC
#define __NR_mknod	0xE
#define __NR_lseek	0x13
#define __NR_setuid	0x17
#define __NR_getuid	0x18
#define __NR_sync	0x24
#define __NR_setgid	0x2E
#define __NR_getgid	0x2F
#define __NR_geteuid	0x31
#define __NR_getegid	0x32
#define __NR_setsid	0x42
#define __NR_chown	0xB6

#endif /* ARCH_UNISTD_H */
