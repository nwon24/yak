#ifndef _STDIO_H
#define _STDIO_H

#ifdef __cplusplus
extern "C" {
#endif

#define EOF	(-1)

#define FOPEN_MAX	20

#define BUFSIZ	1024

#include <bits/decl_FILE.h>
#ifndef __NEED_OFF_T
#define __NEED_OFF_T
#endif
#ifndef __NEED_SIZE_T
#define __NEED_SIZE_T
#endif
#ifndef __NEED_SSIZE_T
#define __NEED_SSIZE_T
#endif
#include <bits/decl_types.h>

#ifndef __DEFINED_OFF_T
typedef __off_t off_t;
#define __DEFINED_OFF_T
#endif
#ifndef __DEFINED_SIZE_T
typedef __size_t size_t;
#define __DEFINED_SIZE_T
#endif
#ifndef __DEFINED_SSIZE_T
typedef __ssize_t ssize_t;
#define __DEFINED_SSIZE_T
#endif

#define __need___v_list
#include <stdarg.h>

extern FILE __file_table[];

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

int fprintf(FILE *restrict stream, const char *restrict format, ...);
int vfprintf(FILE *restrict stream, const char *restrict format, va_list ap);

int fflush(FILE *stream);
int fclose(FILE *stream);
FILE *fopen(const char *restrict name, const char *restrict mode);
size_t fread(void *restrict ptr, size_t size, size_t nmemb, FILE *restrict stream);
size_t fwrite(const void *restrict ptr, size_t size, size_t nmemb, FILE *restrict stream);
void setbuf(FILE *restrict stream, char *restrict buf);
int fseek(FILE *stream, long offset, int whence);

#ifdef __cplusplus
}
#endif

#endif /* _STDIO_H */
