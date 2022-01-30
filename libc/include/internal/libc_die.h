#ifndef _INTERNAL_LIBC_DIE_H
#define _INTERNAL_LIBC_DIE_H

#if defined(__i386__)
#include <i386/libc_die.h>
#else
#error "Arch not supported"
#endif

#endif
