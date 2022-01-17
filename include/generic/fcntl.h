#ifndef FCNTL_H
#define FCNTL_H

#ifndef __ASSEMBLER__

#include <asm/types.h>

int open(const char *path, int flags, ...);

#endif /* __ASSEMBLER__ */

#define O_ACCMODE	03	/* Lower 2 bits of mode */
#define O_RDONLY	00
#define O_WRONLY	01
#define O_RDWR		02
#define O_CREAT		00100
#define O_EXCL		00200
#define O_NOCTTY	00400
#define O_TRUNC		01000
#define O_APPEND	02000

#endif /* FCNTL_H */
