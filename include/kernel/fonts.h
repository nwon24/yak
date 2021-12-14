#ifndef _FONTS_H_
#define _FONTS_H_

#include <stdint.h>

extern unsigned char default8x16_psfu[];
extern unsigned char *font_table[];

/*
 * It may look redundant to have a struct like this so similar
 * to the PSF header, but it is here in case we choose to have
 * another format for bitmap fonts.
 */
struct bitmap_font {
        uint32_t height;
        uint32_t width;
        uint32_t bytes_per_glpyh;
        uint32_t nr_glpyh;
        unsigned char *data;
};

struct bitmap_font *load_default_font(void);

#endif /* FONTS_H_ */
