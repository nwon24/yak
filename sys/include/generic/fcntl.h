#ifndef FCNTL_H
#define FCNTL_H

#ifndef __LIBRARY__

#ifndef __ASSEMBLER__

#include <asm/types.h>

int open(const char *path, int flags, ...);

#endif /* __ASSEMBLER__ */

#endif

#define O_ACCMODE	03	/* Lower 2 bits of mode */
#define O_RDONLY	00
#define O_WRONLY	01
#define O_RDWR		02
#define O_CREAT		00100
#define O_EXCL		00200
#define O_NOCTTY	00400
#define O_TRUNC		01000
#define O_APPEND	02000
#define O_NONBLOCK	04000

#define F_DUPFD		0
#define F_DUPFD_CLOEXEC	1
#define F_GETFD		2
#define F_SETFD		3
#define F_GETFL		4
#define F_SETFL		5
#define F_GETLK		6
#define F_SETLK		7
#define F_SETLKW	8
#define F_SETOWN	9
#define F_GETOWN	10
#define F_SETSIG	11
#define F_GETSIG	12

#define FD_CLOEXEC	1

#endif /* FCNTL_H */
