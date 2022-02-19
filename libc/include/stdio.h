#ifndef _STDIO_H
#define _STDIO_H

#ifdef __cplusplus
extern "C" {
#endif

#define EOF	(-1)

#define OPEN_MAX	20

#include <bits/decl_FILE.h>

extern FILE __io_open[];

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

int fprintf(FILE *restrict stream, const char *restrict format, ...);
int vfprintf(FILE *restrict stream, const char *restrict format, va_list ap);

int fflush(FILE *stream);
int fclose(FILE *stream);
FILE *fopen(const char *restrict name, const char *restrict mode, ...);
size_t fread(void *restrict ptr, size_t size, size_t nmemb, FILE *restrict stream);
size_t fwrite(void *restrict ptr, size_t size, size_t nmemb, FILE *restrict stream);
void setbuf(FILE *restrict stream, char *restrict buf);
int fseek(FILE *stream, long offset, int whence);

#ifdef __cplusplus
}
#endif

#endif /* _STDIO_H */
