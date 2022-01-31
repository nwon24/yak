#ifndef _SYS_STAT_H
#error "<bits/decl_struct_timespec.h> should not be included directly"
#endif

#if !defined(__DEFINED_TIME_T)
typedef __time_t time_t;
#define __DEFINED_TIME_T
#endif

#if !defined(__DEFINED___STRUCT_TIMESPEC)
struct __timespec {
	time_t tv_sec;
	long tv_nsec;
};
#define __DEFINED___STRUCT_TIMESPEC
#endif
