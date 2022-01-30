#ifndef _UNISTD_H
#define _UNISTD_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <bits/decl_types.h>

#include <bits/unistd.h>

/* A lie */
#define _POSIX_VERSION	200809L

#define STDIN_FILENO	0
#define STDOUT_FILENO	1
#define STDERR_FILENO	2

#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2

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
#ifndef __DFINED_GID_T
typedef __gid_t gid_t;
#define __DEFINED_GID_T
#endif
#ifndef __DEFINED_PID_T
typedef __pid_t pid_t;
#define __DEFINED_PID_T
#endif
#ifndef __SIZE_T_DEFINED
typedef __size_t size_t;
#endif
#ifndef __SSIZE_T_DEFINED
typedef __ssize_t ssize_t;
#endif
typedef __off_t off_t;

ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, void *buf, size_t count);
int close(int fd);
int execve(const char *path, const char *argv[], const char *envp[]);
pid_t fork(void);
void _exit(int status);
off_t lseek(int fd, off_t off, int whence);
void sync(void);
int dup(int fd);
int dup2(int oldfd, int newfd);

#endif /* __ASSEMBLER__ */

#if defined(__cplusplus)
}
#endif

#endif /* _UNISTD_H */
