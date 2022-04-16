#ifndef __BITS_STDIO_H
#define __BITS_STDIO_H

int __fillbuf(FILE *fp);
void __flushbuf(FILE *fp, int c);

void __libc_stdio_init(void);
void __libc_stdio_fini(void);

#define __getc_internal(fp)	__getc_unlocked(fp)
#define __putc_internal(c, fp)	__putc_unlocked(c, fp)

#define __getc_unlocked(fp) \
	(--((fp)->rcount) > 0 ? *(unsigned char *)((fp)->rptr++) : __fillbuf((fp)))
#define __putc_unlocked(c, fp)			\
	((fp)->wcount++ < __FILE_BUFSIZ ? *((fp)->wptr++) = (c) : __flushbuf((fp), (c)))

#endif /* __BITS_STDIO_H */
