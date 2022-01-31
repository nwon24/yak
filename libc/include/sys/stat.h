#ifndef _SYS_STAT_H
#define _SYS_STAT_H

#ifndef __NEED_BLKCNT_T
#define __NEED_BLKCNT_T
#endif
#ifndef __NEED_BLKSIZE_T
#define __NEED_BLKSIZE_T
#endif
#ifndef __NEED_DEV_T
#define __NEED_DEV_T
#endif
#ifndef __NEED_INO_T
#define __NEED_INO_T
#endif
#ifndef __NEED_MODE_T
#define __NEED_MODE_T
#endif
#ifndef __NEED_NLINK_T
#define __NEED_NLINK_T
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
#ifndef __NEED_TIME_T
#define __NEED_TIME_T
#endif
#ifndef __NEED_STRUCT_TIMESPEC
#define __NEED_STRUCT_TIMESPEC
#endif

#include <bits/decl_types.h>

#ifndef __ASSEMBLER__

#if !defined(__DEFINED_BLKCNT_T)
typedef __blkcnt_t blkcnt_t;
#define __DEFINED_BLKCNT_T
#endif

#if !defined(__DEFINED_BLKSIZE_T)
typedef __blksize_t blksize_t;
#define __DEFINED_BLKSIZE_T
#endif

#if !defined(__DEFINED_DEV_T)
typedef __dev_t dev_t;
#define __DEFINED_DEV_T
#endif

#if !defined(__DEFINED_INO_T)
typedef __ino_t ino_t;
#define __DEFINED_INO_T
#endif

#if !defined(__DEFINED_MODE_T)
typedef __mode_t mode_t;
#define __DEFINED_MODE_T
#endif

#if !defined(__DEFINED_NLINK_T)
typedef __nlink_t nlink_t;
#define __DEFINED_NLINK_T
#endif

#if !defined(__DEFINED_UID_T)
typedef __uid_t uid_t;
#define __DEFINED_UID_T
#endif

#if !defined(__DEFINED_GID_T)
typedef __gid_t gid_t;
#define __DEFINED_GID_T
#endif

#if !defined(__DEFINED_TIME_T)
typedef __time_t time_t;
#define __DEFINED_TIME_T
#endif

#if !defined(__DEFINED_OFF_T)
typedef __off_t off_t;
#define __DEFINED_OFF_T
#endif

#if !defined(__DEFINED_STRUCT_TIMESPEC)
#include <bits/decl_struct_timespec.h>
#define timespec	__timespec
#define __DEFINED_STRUCT_TIMESPEC
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

#define __DEFINED_MODE_T_FILE_BITS
#endif

#define S_IFBLK		__S_IFBLK
#define S_IFCHR		__S_IFCHR
#define S_IFIFO		__S_IFIFO
#define S_IFREG		__S_IFREG
#define S_IFDIR		__S_IFDIR
#define S_IFLNK		__S_IFLNK
#define S_IFSOCK	__S_IFSOCK

#define S_ISBLK(m)	__S_ISBLK(m)
#define S_ISCHR(m)	__S_ISCHR(m)
#define S_ISFIFO(m)	__S_ISFIFO(m)
#define S_ISREG(m)	__S_ISREG(m)
#define S_ISDIR(m)	__S_ISDIR(m)
#define S_ISLNK(m)	__S_ISLNK(m)
#define S_ISSOCK(m)	__S_ISSOCK(m)

struct stat {
	dev_t st_dev;
	ino_t st_ino;
	mode_t st_mode;
	nlink_t st_nlink;
	uid_t st_uid;
	gid_t st_gid;
	dev_t st_rdev;
	off_t st_size;
	struct timespec st_atim;
	struct timespec st_mtim;
	struct timespec st_ctim;
	blksize_t st_blksize;
	blkcnt_t st_block;
};

int chmod(const char *path, mode_t mode);
int stat(const char *restrict path, struct stat *restrict statbuf);
int mkdir(const char *path, mode_t mode);
int mknod(const char *path, mode_t mode, dev_t dev);
mode_t umask(mode_t cmask);
int fstat(int fd, struct stat *statbuf);
int lstat(const char *restrict path, struct stat *restrict statbuf);

#endif /* __ASSEMBLER__ */

#endif /* _SYS_STAT_H */
