#ifndef __ASSEMBLER__

#ifndef __NEED_INT32_T
#define __NEED_INT32_T
#endif
#ifndef __NEED_UINT32_T
#define __NEED_UINT32_T
#endif

#if defined(__i386__)
#include <bits/decl_int_types_32.h>
#else
#error "Unsupported 64-bit arch"
#endif

#if defined(__NEED_PID_T) && !defined(__DEFINED___PID_T)
typedef int __pid_t;
#define __DEFINED___PID_T
#endif
#if defined(__NEED_MODE_T) && !defined(__DEFINED___MODE_T)
typedef int __mode_t;
#define __DEFINED___MODE_T
#endif
#if defined(__NEED_DEV_T) && !defined(__DEFINED___DEV_T)
typedef int __dev_t;
#define __DEFINED___DEV_T
#endif
#if defined(__NEED_NLINK_T) && !defined(__DEFINED___NLINK_T)
typedef int __nlink_t;
#define __DEFINED___NLINK_T
#endif
#if defined(__NEED_UID_T) && !defined(__DEFINED___UID_T)
typedef int __uid_t;
#define __DEFINED___UID_T
#endif
#if defined(__NEED_ID_T) && !defined(__DEFINED___ID_T)
typedef int __id_t;
#define __DEFINED___ID_T
#endif
#if defined(__NEED_GID_T) && !defined(__DEFINED___GID_T)
typedef int __gid_t;
#define __DEFINED___GID_T
#endif
#if defined(__NEED_BLKCNT_T) && !defined(__DEFINED___BLKCNT_T)
typedef signed int __blkcnt_t;
#define __DEFINED___BLKCNT_T
#endif
#if defined(__NEED_BLKSIZE_T) && !defined(__DEFINED___BLKSIZE_T)
typedef signed int __blksize_t;
#define __DEFINED___BLKSIZE_T
#endif
#if defined(__NEED_OFF_T) && !defined(__DEFINED___OFF_T)
typedef signed int __off_t;
#define __DEFINED___OFF_T
#endif
#if defined(__NEED_FSBLKCNT_T) && !defined(__DEFINED___FSBLKCNT_T)
typedef unsigned __fsblkcnt_t;
#define __DEFINED___FSBLKCNT_T
#endif
#if defined(__NEED_FSFILCNT_T) && !defined(__DEFINED___FSFILCNT_T)
typedef unsigned int __fsfilcnt_t;
#define __DEFINED___FSFILCNT_T
#endif
#if defined(__NEED_INO_T) && !defined(__DEFINED___INO_T)
typedef unsigned int __ino_t;
#define __DEFINED___INO_T
#endif
#if defined(__NEED_TIME_T) && !defined(__DEFINED___TIME_T)
typedef long __time_t;
#define __DEFINED___TIME_T
#endif
#if defined(__i386__)

#if !defined(__DEFINED___SIZE_T) && defined(__NEED_SIZE_T)
typedef __uint32_t __size_t;
#define __DEFINED___SIZE_T
#endif

#if !defined(__DEFINED___SSIZE_T) && defined(__NEED_SSIZE_T)
typedef __int32_t __ssize_t;
#define __DEFINED___SSIZE_T
#endif

#elif defined(__x86_64__)

#if !defined(__DEFINED___SIZE_T) && defined(__NEED_SIZE_T)
typedef __uint64_t __size_t;
#define __DEFINED___SIZE_T
#endif
#if !defined(__DEFINED___SSIZE_T) && defined(__NEED_SSIZE_T)
typedef __int64_t __ssize_t;
#define __DEFINED___SIZE_T
#endif

#else
#error "Unsupported arch"
#endif

#endif /* __ASSEMBLER__ */
