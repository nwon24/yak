#ifndef _BITS_DECL_TYPES_H
#define _BITS_DECL_TYPES_H

#ifndef __ASSEMBLER__

#if defined(__i386__)
#include <bits/decl_int_types_32.h>
#else
#error "Unsupported 64-bit arch"
#endif

#include <stddef.h>
#define __SIZE_T_DEFINED

typedef int __pid_t;
typedef int __mode_t;
typedef int __dev_t;
typedef int __nlink_t;
typedef int __uid_t;
typedef int __id_t;
typedef int __gid_t;
typedef signed int __blkcnt_t;
typedef signed int __off_t;
typedef unsigned __fsblkcnt_t;
typedef unsigned int __fsfilcnt_t;
typedef unsigned int __ino_t;
#if defined(__i386__)
#if !defined(__SIZE_T_DEFINED)
typedef __uint32_t __size_t;
#endif
typedef __int32_t __ssize_t;
#elif defined(__x86_64__)
#if !defined(__SIZE_T_DEFINED)
typedef __uint64_t __size_t;
#endif
typedef __int64_t __ssize_t;
#else
#error "Unsupported arch"
#endif

#endif /* __ASSEMBLER__ */

#endif /* _BITS_DECL_TYPES_H */
