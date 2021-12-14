/*
 * psf.c
 * Code to work with PC Screen Fonts.
 */

#include <stddef.h>

#include <drivers/psf.h>

#include <kernel/debug.h>
#include <kernel/fonts.h>

static struct bitmap_font loaded_font;

struct bitmap_font *
load_default_font(void)
{
        struct psf_header *psf;

        psf = (struct psf_header *)font_table[0];
        if (psf->magic != PSF_HEADER_MAGIC) {
                printk("Loaded PSF bitmap font invalid");
                return NULL;
        }
        if (psf->width > 32) {
                printk("Invalid PSF width %d\r\n", psf->width);
                return NULL;
        }
        loaded_font.height = psf->height;
        loaded_font.width = psf->width;
        loaded_font.bytes_per_glpyh = psf->bytes_per_glpyh;
        loaded_font.nr_glpyh = psf->numglyph;
        loaded_font.data = (unsigned char *)psf + psf->headersize;
        return &loaded_font;
}
