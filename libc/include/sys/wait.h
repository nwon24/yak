#ifndef _SYS_WAIT_H
#define _SYS_WAIT_H

#if defined(__cplusplus)
extern "C" {
#endif

#define __NEED_PID_T
#define __NEED_ID_T

#include <bits/decl_types.h>

#if !defined(__DEFINED_PID_T)
typedef __pid_t pid_t;
#define __DEFINED_PID_T
#endif
#if !defined(__DEFINED_ID_T)
typedef __id_t id_t;
#define __DEFINED_ID_T
#endif

#define WNOHANG         1
#define WUNTRACED	2
#define WSTOPPED	2
#define WEXITED		4
#define WCONTINUED	8
#define WNOWAIT		0x1000000

#define WEXITSTATUS(s)	(((s) & 0xFF00) >> 8)
#define WTERMSIG(s)	((s) & 0x7F)
#define WSTOPSIG(s)	WEXITSTATUS(s)
#define WIFEXITED(s)	(WEXITSTATUS(s) == 0)

#define WIFCONTINUED(s)	((s) == 0xFFFF)
#define WIFSIGNALED(s)	\
	(((signed char) (((s) & 0x7F) + 1) >> 1) > 0)

typedef enum {
	P_ALL = 0,
	P_PID = 1,
	P_PGID = 2,
	P_PIDFD = 3,
} idtype_t;

pid_t wait(int *wstatus);
pid_t waitpid(pid_t, int *wstatus, int options);

#if defined(__cplusplus)
}
#endif

#endif /* _WAIT_H */
