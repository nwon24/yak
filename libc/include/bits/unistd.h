#ifndef _BITS_UNISTD_H
#define _BITS_UNISTD_H

#include <hobbix/asm/unistd.h>

#define SYS_exit	__NR_exit
#define SYS_fork	__NR_fork
#define SYS_read	__NR_read
#define SYS_write	__NR_write
#define SYS_open	__NR_open
#define SYS_close	__NR_close
#define SYS_waitpid	__NR_waitpid
#define SYS_creat	__NR_creat
#define SYS_link	__NR_link
#define SYS_unlink	__NR_unlink
#define SYS_execve	__NR_execve
#define SYS_chdir	__NR_chdir
#define SYS_time	__NR_time
#define SYS_mknod	__NR_mknod
#define SYS_chmod	__NR_chmod
#define SYS_lchown	__NR_lchown
#define SYS_lseek	__NR_lseek
#define SYS_getpid	__NR_getpid
#define SYS_setuid	__NR_setuid
#define SYS_getuid	__NR_getuid
#define SYS_stime	__NR_stime
#define SYS_alarm	__NR_alarm
#define SYS_pause	__NR_pause
#define SYS_access	__NR_access
#define SYS_nice	__NR_nice
#define SYS_sync	__NR_sync
#define SYS_kill	__NR_kill
#define SYS_rename	__NR_rename
#define SYS_mkdir	__NR_mkdir
#define SYS_rmdir	__NR_rmdir
#define SYS_dup		__NR_dup
#define SYS_setgid	__NR_setgid
#define SYS_getgid	__NR_getgid
#define SYS_signal	__NR_signal
#define SYS_geteuid	__NR_geteuid
#define SYS_getegid	__NR_getegid
#define SYS_fcntl	__NR_fcntl
#define SYS_setpgid	__NR_setpgid
#define SYS_umask	__NR_umask
#define SYS_chroot	__NR_chroot
#define SYS_dup2	__NR_dup2
#define SYS_getppid	__NR_getppid
#define SYS_getpgrp	__NR_getpgrp
#define SYS_setsid	__NR_setsid
#define SYS_symlink	__NR_symlink
#define SYS_fchmod	__NR_fchmod
#define SYS_stat	__NR_stat
#define SYS_lstat	__NR_lstat
#define SYS_fstat	__NR_fstat
#define SYS_getpgid	__NR_getpgid
#define SYS_getsid	__NR_getsid
#define SYS_chown	__NR_chown

#endif /* _BITS_UNISTD_H */
