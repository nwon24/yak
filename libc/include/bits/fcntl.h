#ifndef _BITS_FCNTL_H
#define _BITS_FCNTL_h

#include <hobbix/generic/fcntl.h>

#include <bits/stat.h>

#ifndef O_ACCMODE
#define O_ACCMODE	003
#endif
#ifndef O_RDONLY
#define O_RDONLY	000
#endif
#ifndef O_WRONLY
#define O_WRONLY	001
#endif
#ifndef O_RDWR
#define O_RDWR		002
#endif
#ifndef O_CREAT
#define O_CREAT		00100
#endif
#ifndef O_EXCL
#define O_EXCL		00200
#endif
#ifndef O_NOCTTY
#define O_NOCTTY	00400
#endif
#ifndef O_TRUNC
#define O_TRUNC		01000
#endif
#ifndef O_APPEND
#define O_APPEND	02000
#endif
#ifndef O_NONBLOCK
#define O_NONBLOCK	04000
#endif

#ifndef F_DUPFD
#define F_DUPFD		1
#endif
#ifndef F_GETFD
#define F_GETFD		2
#endif
#ifndef F_SETFD
#define F_SETFD		3
#endif
#ifndef F_GETFL
#define F_GETFL		4
#endif
#ifndef F_GETLK
#define F_GETLK		5
#endif
#ifndef F_SETLK
#define F_SETLK		6
#endif
#ifndef F_SETLKW
#define F_SETLKW	7
#endif
#ifndef F_GETOWN
#define F_GETOWN	8
#endif
#ifndef F_SETOWN
#define F_SETOWN	9
#endif
#ifndef F_DUPFD_CLOEXEC
#define F_DUPFD_CLOEXEC 1030
#endif

#ifndef FD_CLOEXEC
#define FD_CLOEXEC	1
#endif

#if !defined(__DEFINED_MODE_T_FILE_BITS)

#define S_IRWXU		__S_IRWXU
#define S_IRUSR		__S_IRUSR
#define S_IWUSR		__S_IWUSR
#define S_IXUSR		__S_IXUSR
#define S_IRWXG		__S_IRWXG
#define S_IRGRP		__S_IRGRP
#define S_IWGRP		__S_IWGRP
#define S_IXGRP		__S_IXGRP
#define S_IRWXO		__S_IRWXO
#define S_IROTH		__S_IROTH
#define S_IWOTH		__S_IWOTH
#define S_IXOTH		__S_IXOTH
#define S_ISUID		__S_ISUID
#define S_ISGID		__S_ISGID
#define S_ISVTX		__S_ISVTX
#define S_ISUID		__S_ISUID
#define S_ISGID		__S_ISGID
#define S_ISVTX		__S_ISVTX

#define __DEFINED_MODE_T_FILE_BITS

#endif

#endif
