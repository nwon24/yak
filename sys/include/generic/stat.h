#ifndef STAT_H
#define STAT_H

#define S_IFMT		0170000
#define S_IFIFO		0010000
#define S_IFCHR		0020000
#define S_IFDIR		0040000
#define S_IFBLK		0060000
#define S_IFREG		0100000
#define S_IFLNK		0120000
#define S_IFSOCK	0140000

#define S_ISFIFO(m)	(((m) & S_IFMT) == S_IFIFO)
#define S_ISCHR(m)	(((m) & S_IFMT) == S_IFCHR)
#define S_ISDIR(m)	(((m) & S_IFMT) == S_IFDIR)
#define S_ISBLK(m)	(((m) & S_IFMT) == S_IFBLK)
#define S_ISREG(m)	(((m) & S_IFMT) == S_IFREG)
#define S_ISLNK(m)	(((m) & S_IFMT) == S_IFLNK)
#define S_ISSOCK(m)	(((m) & S_IFMT) == S_IFSOCK)

#define S_IXOTH		00001
#define S_IWOTH		00002
#define S_IROTH		00004
#define S_IRWXO		(S_IROTH | S_IWOTH | S_IXOTH)

#define S_IXGRP		000010
#define S_IWGRP		000020
#define S_IRGRP		000040
#define S_IRWXG		(S_IRGRP | S_IWGRP | S_IXGRP)

#define S_IXUSR		000100
#define S_IWUSR		000200
#define S_IRUSR		000400
#define S_IRWXU		(S_IRUSR | S_IWUSR | S_IXUSR)

#define S_ISVTX		001000
#define S_ISGID		002000
#define S_ISUID		004000

#ifndef __ASSEMBLER__

#include <asm/types.h>

struct stat {
	dev_t st_dev;
	ino_t st_ino;
	mode_t st_mode;
	nlink_t st_nlink;
	uid_t st_uid;
	gid_t st_gid;
	dev_t st_rdev;
	off_t st_size;
	time_t st_atime;
	time_t st_mtime;
	time_t st_ctime;
	blksize_t st_blksize;
	blkcnt_t st_blocks;
};

#endif /* __ASSEMBLER__*/
#endif /* STAT_H */
