#ifndef _STDLIB_H
#define _STDLIB_H

#ifndef __NEED_SIZE_T
#define __NEED_SIZE_T
#endif
#include <bits/decl_types.h>

#if !defined(__DEFINED_SIZE_T)
typedef __size_t size_t;
#define __DEFINED_SIZE_T
#endif

#define EXIT_SUCCESS	0
#define EXIT_FAILURE	1

void abort(void);
void *malloc(size_t size);
void *calloc(size_t nmemb, size_t size);
void free(void *ptr);

void exit(int status);
int atexit(void (*)(void));
int atoi(const char *nptr);
char *getenv(const char *name);

#endif /* _STDLIB_H */
