#ifndef FCNTL_H
#define FCNTL_H

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
#define F_GETFD		1
#define F_SETFD		2
#define F_GETFL		3
#define F_SETFL		4
#define F_GETLK		5
#define F_SETLK		6
#define F_SETLKW	7
#define F_SETOWN	8
#define F_GETOWN	9
#define F_SETSIG	10
#define F_GETSIG	11

#define F_DUPFD_CLOEXEC	1030

#define FD_CLOEXEC	1

#endif /* FCNTL_H */
