#ifndef SIGNAL_H
#define SIGNAL_H

#define NSIG		32

#define SIGHUP		1
#define SIGINT		2
#define SIGQUIT		3
#define SIGILL		4
#define SIGTRAP		5
#define SIGABRT		6
#define SIGBUS		7
#define SIGFPE		8
#define SIGKILL		9
#define SIGCHLD		10
#define SIGSEGV		11
#define SIGCONT		12
#define SIGPIPE		13
#define SIGALRM		14
#define SIGTERM		15
#define SIGPOLL		16
#define SIGPROF		17
#define SIGSTOP		18
#define SIGSYS		19
#define SIGTSTP		20
#define SIGTTIN		21
#define SIGTTOU		22
#define SIGUSR1		23
#define SIGUSR2		24
#define SIGURG		25
#define SIGVTALRM	26
#define SIGXCPU		27
#define SIGXFSZ		28
#define SIGWINCH	29

typedef void (*sighandler_t)(int);

/* 0 and 1 are not valid addresses of functions (hopefully) */
#define SIG_DFL		((sighandler_t)0)
#define SIG_IGN		((sighandler_t)1)

#define SIG_ERR		((sighandler_t)-1)

#endif /* SIGNAL_H */
