#ifndef ARCH_UNISTD_H
#define ARCH_UNISTD_H

#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2

#define __NR_fork	0x2
#define __NR_read	0x3
#define __NR_write	0x4
#define __NR_open	0x5
#define __NR_creat	0x8
#define __NR_link	0x9
#define __NR_unlink	0xA
#define __NR_lseek	0x13
#define __NR_sync	0x24

#endif /* ARCH_UNISTD_H */
