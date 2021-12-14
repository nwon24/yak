#ifndef _PSF_H_
#define _PSF_H_

#include <stdint.h>

#define PSF_HEADER_MAGIC	0x864AB572

struct psf_header {
        uint32_t magic;
        uint32_t version;
        uint32_t headersize;
        uint32_t flags;
        uint32_t numglyph;
        uint32_t bytes_per_glpyh;
        uint32_t height;
        uint32_t width;
};

#endif /* PSF_H_ */
