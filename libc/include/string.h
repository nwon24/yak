#ifndef _STRING_H
#define _STRING_H

#ifndef __NEED_SIZE_T
#define __NEED_SIZE_T
#endif
#include <bits/decl_types.h>

#if !defined(__DEFINED_SIZE_T)
typedef __size_t size_t;
#define __DEFINED_SIZE_T
#endif

void *memcpy(void *restrict dest, const void *restrict src, size_t n);
void *memmove(void *restrict dest, const void *restrict src, size_t n);
void *memset(void *s, int c, size_t n);

size_t strlen(const char *s);
char *strcpy(char *restrict dest, const char *src);

#endif
