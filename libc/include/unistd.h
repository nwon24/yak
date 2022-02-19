#ifndef _UNISTD_H
#define _UNISTD_H

#if defined(__cplusplus)
extern "C" {
#endif

#ifndef __NEED_SIZE_T
#define __NEED_SIZE_T
#endif
#ifndef __NEED_SSIZE_T
#define __NEED_SSIZE_T
#endif
#ifndef __NEED_UID_T
#define __NEED_UID_T
#endif
#ifndef __NEED_GID_T
#define __NEED_GID_T
#endif
#ifndef __NEED_OFF_T
#define __NEED_OFF_T
#endif
#ifndef __NEED_PID_T
#define __NEED_PID_T
#endif
#ifndef __NEED_INTPTR_T
#define __NEED_INTPTR_T
#endif

#include <bits/decl_types.h>

#include <bits/unistd.h>

/* A lie */
#define _POSIX_VERSION	200809L

#define STDIN_FILENO	0
#define STDOUT_FILENO	1
#define STDERR_FILENO	2

#ifndef SEEK_SET
#define SEEK_SET	0
#endif
#ifndef SEEK_CUR
#define SEEK_CUR	1
#endif
#ifndef SEEK_END
#define SEEK_END	2
#endif

#define F_OK	0
#define X_OK	1
#define W_OK	2
#define R_OK	4

#ifndef NULL
#define NULL	((void *)0)
#endif

#ifndef __ASSEMBLER__

#ifndef __DEFINED_UID_T
typedef __uid_t uid_t;
#define __DEFINED_UID_T
#endif
#ifndef __DEFINED_GID_T
typedef __gid_t gid_t;
#define __DEFINED_GID_T
#endif
#ifndef __DEFINED_PID_T
typedef __pid_t pid_t;
#define __DEFINED_PID_T
#endif
#ifndef __DEFINED_SIZE_T
typedef __size_t size_t;
#define __DEFINED_SIZE_T
#endif
#ifndef __DEFINED_SSIZE_T
typedef __ssize_t ssize_t;
#define __DEFINED_SSIZE_T
#endif
#ifndef __DEFINED_OFF_T
typedef __off_t off_t;
#define __DEFINED_OFF_T
#endif
#ifndef __DEFINED_INTPTR_T
typedef __intptr_t inptr_t;
#define __DEFINED_INTPTR_T
#endif

int access(const char *path, int mode);
unsigned int alarm(unsigned int seconds);
int chdir(const char *path);
int chown(const char *path, uid_t uid, gid_t gid);
int lchown(const char *path, uid_t uid, gid_t gid);
gid_t getegid(void);
uid_t geteuid(void);
gid_t getgid(void);
pid_t getpgid(pid_t pid);
pid_t getpgrp(void);
pid_t getpid(void);
pid_t getppid(void);
pid_t getsid(pid_t pid);
uid_t getuid(void);
int setpgid(pid_t pid, pid_t pgid);
pid_t setsid(void);
int setuid(uid_t uid);
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, void *buf, size_t count);
int close(int fd);
int execve(const char *path, const char *argv[], const char *envp[]);
int execv(const char *path, char *const argv[]);
int execvp(const char *file, char *const argv[]);
pid_t fork(void);
void _exit(int status);
off_t lseek(int fd, off_t off, int whence);
void sync(void);
int dup(int fd);
int dup2(int oldfd, int newfd);
int link(const char *path1, const char *path2);
int unlink(const char *path);
int symlink(const char *path1, const char *path2);
int rmdir(const char *path);
int nice(int inc);

#endif /* __ASSEMBLER__ */

#if defined(__cplusplus)
}
#endif

#endif /* _UNISTD_H */
