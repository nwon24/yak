#ifndef _BITS_STAT_H
#define _BITS_STAT_H

#ifndef __DEFINED__FILE_MODE_BITS

#define __S_IRWXU	0700
#define __S_IRUSR	0400
#define __S_IWUSR	0200
#define __S_IXUSR	0100
#define __S_IRWXG	0070
#define __S_IRGRP	0040
#define __S_IWGRP	0020
#define __S_IXGRP	0010
#define __S_IRWXO	0007
#define __S_IROTH	0004
#define __S_IWOTH	0002
#define __S_IXOTH	0001
#define __S_ISUID	04000
#define __S_ISGID	02000
#define __S_ISVTX	01000

#define __S_IFMT	0170000
#define __S_IFSOCK	0140000
#define __S_IFLNK	0120000
#define __S_IFREG	0100000
#define __S_IFBLK	0060000
#define __S_IFDIR	0040000
#define __S_IFCHR	0020000
#define __S_IFIFO	0010000

#define __S_ISFMT(m, fmt)	((m) & __S_IFMT == (fmt))
#define __S_ISSOCK(m)	__S_ISFMT(m, __S_IFSOCK)
#define __S_ISLNK(m)	__S_ISFMT(m, __S_IFLNK)
#define __S_ISREG(m)	__S_ISFMT(m, __S_IFREG)
#define __S_ISBLK(m)	__S_ISFMT(m, __S_IFBLK)
#define __S_ISDIR(m)	__S_ISFMT(m, __S_IFDIR)
#define __S_ISCHR(m)	__S_ISFMT(m, __S_IFCHR)
#define __S_ISFIFO(m)	__S_ISFMT(m, __S_IFIFO)

#define __DEFINED__FILE_MODE_BITS
#endif

#endif /* _BITS_STAT_H */
