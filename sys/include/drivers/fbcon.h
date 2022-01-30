#ifndef FBCON_H_
#define FBCON_H_

#include <drivers/fb.h>
#include <kernel/fonts.h>

struct fbcon {
        struct fb_info *fbcon_info;
        struct bitmap_font *fbcon_font;
        uint32_t fbcon_fg;
        uint32_t fbcon_bg;
};

void fbcon_init(struct fb_info *info);

#endif /* FBCON_H_*/
