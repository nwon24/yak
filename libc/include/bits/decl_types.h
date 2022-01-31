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

#ifdef __NEED_PID_T
typedef int __pid_t;
#endif
#ifdef __NEED_MODE_T
typedef int __mode_t;
#endif
#ifdef __NEED_DEV_T
typedef int __dev_t;
#endif
#ifdef __NEED_NLINK_T
typedef int __nlink_t;
#endif
#ifdef __NEED_UID_T
typedef int __uid_t;
#endif
#ifdef __NEED_ID_T
typedef int __id_t;
#endif
#ifdef __NEED_GID_T
typedef int __gid_t;
#endif
#ifdef __NEED_BLKCNT_T
typedef signed int __blkcnt_t;
#endif
#ifdef __NEED_OFF_T
typedef signed int __off_t;
#endif
#ifdef __NEED_FSBLKCNT_T
typedef unsigned __fsblkcnt_t;
#endif
#ifdef __NEED_FSFILCNT_T
typedef unsigned int __fsfilcnt_t;
#endif
#ifdef __NEED_INO_T
typedef unsigned int __ino_t;
#endif
#if defined(__i386__)
#if !defined(__SIZE_T_DEFINED) && defined(__NEED_SIZE_T)
typedef __uint32_t __size_t;
#endif
typedef __int32_t __ssize_t;
#elif defined(__x86_64__)
#if !defined(__SIZE_T_DEFINED) && defined(__NEED_SSIZE_T)
typedef __uint64_t __size_t;
#endif
typedef __int64_t __ssize_t;
#else
#error "Unsupported arch"
#endif

#endif /* __ASSEMBLER__ */

#endif /* _BITS_DECL_TYPES_H */
