#ifndef _FCNTL_H
#define _FCNTL_H

#ifndef __NEED_MODE_T
#define __NEED_MODE_T
#endif
#ifndef __NEED_OFF_T
#define __NEED_OFF_T
#endif
#ifndef __NEED_PID_T
#define __NEED_PID_T
#endif

#include <bits/fcntl.h>

#include <bits/decl_types.h>

#ifndef SEEK_SET
#define SEEK_SET	0
#endif
#ifndef SEEK_CUR
#define SEEK_CUR	1
#endif
#ifndef SEEK_END
#define SEEK_END	2
#endif

#ifndef __ASSEMBLER__

#if !defined(__DEFINED_MODE_T)
typedef __mode_t mode_t;
#define __DEFINED_MODE_T
#endif
#if !defined(__DEFINED_OFF_T)
typedef __off_t off_t;
#define __DEFINED_OFF_T
#endif
#if !defined(__DEFINED_PID_T)
typedef __pid_t pid_t;
#define __DEFINED_PID_T
#endif

int open(const char *path, int mode, ...);
int creat(const char *path, mode_t mode);

#endif

#endif
