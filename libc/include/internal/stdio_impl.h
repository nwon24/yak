#ifndef __STDIO_IMPL_H
#define __STDIO_IMPL_H

int __fillbuf(FILE *fp);
void __flushbuf(FILE *fp, int c);

void __libc_stdio_init(void);
void __libc_stdio_fini(void);

#define __getc_unlocked(fp)	__internal_getc(fp)
#define __putc_unlocked(fp)	__internal_putc(fp)

#define __internal_getc(fp) \
	(--((fp)->rcount) > 0 ? *(unsigned char *)((fp)->rptr++) : __fillbuf((fp)))
#define __internal_putc(c, fp)			\
	((fp)->wcount++ < __FILE_BUFSIZ ? *((fp)->wptr++) = (c) : __flushbuf((fp), (c)))

#endif /* __STDIO_IMPL_H */
