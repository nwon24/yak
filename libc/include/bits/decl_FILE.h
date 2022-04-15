#define __READ	(1 << 0)
#define __WRITE	(1 << 1)
#define __EOF	(1 << 2)
#define __ERR	(1 << 3)

#define __FILE_BUFSIZ	BUFSIZ

struct __FILE {
	int flags;
	int fd;
	int rcount;
	int wcount;
	char rbuf[__FILE_BUFSIZ];
	char wbuf[__FILE_BUFSIZ];
	char *rptr;
	char *wptr;
};

typedef struct __FILE FILE;
