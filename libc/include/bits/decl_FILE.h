#define __READ	1
#define __WRITE	2

struct __FILE {
	int flags;
	int fd;
	int rcount;
	int wcount;
	char rbuf[BUFSIZ];
	char wbuf[BUFSIZ];
	char *rptr;
	char *wptr;
};

typedef struct __FILE FILE;
