/*
 * font_table.c
 * Contains a table of pointer to font(s).
 */

#include <kernel/config.h>
#include <kernel/fonts.h>

unsigned char *font_table[] = {
#ifdef _CONFIG_FONT_8X16_PSFU
        default8x16_psfu,
#else
#error "No support for another font currently"
#endif /* _CONFIG_FONT_8X16_PSFU */
};
