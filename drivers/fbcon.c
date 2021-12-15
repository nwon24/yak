/*
 * fbcon.c
 * Framebuffer console driver.
 */

#include <stddef.h>

#include <drivers/ansi_vt.h>
#include <drivers/fb.h>
#include <drivers/fbcon.h>
#include <drivers/vbe.h>
#include <drivers/virtual_console.h>

#include <kernel/debug.h>

static struct fbcon fb_console;
static struct fbcon *current_fbcon;

static int fbcon_putc(uint32_t cx, uint32_t cy, int c);
static void fbcon_putc_32bpp(struct bitmap_font *font, unsigned char *data, uint32_t pitch, uint32_t bg, uint32_t fg, uint32_t dst);
static void fbcon_putc_16bpp(struct bitmap_font *font, unsigned char *data, uint32_t pitch, uint16_t bg, uint16_t fg, uint32_t dst);
static void fbcon_blank(uint32_t cx, uint32_t cy);
static void fbcon_blank_16bpp(struct bitmap_font *font, uint32_t pitch, uint32_t dst);
static void fbcon_blank_32bpp(struct bitmap_font *font, uint32_t pitch, uint32_t dst);
static void fbcon_get_dimensions(uint32_t *width, uint32_t *height);
static void fbcon_set_attr(struct vc_attr *attr, enum vc_attr_type type);
static void fbcon_set_attr_32bpp(struct vc_attr *attr, enum vc_attr_type type);
static void fbcon_set_attr_16bpp(struct vc_attr *attr, enum vc_attr_type type);

static uint32_t ansi_colors_bright_fg_32rgb[] = {
        0x181818,	/* Black */
        0xFF0000,	/* Red */
        0x00FF00,	/* Green */
        0xFFFF00,	/* Yellow */
        0x0000FF,	/* Blue */
        0xFF00FF,	/* Magenta */
        0x00FFFF,	/* Cyan */
        0xFFFFFF,	/* White */
        0x000000,	/* Unused */
        0xD3D3D3,	/* Default */
};

static uint32_t ansi_colors_bright_bg_32rgb[] = {
        0x181818,
        0xFF0000,
        0x00FF00,
        0xFFFF00,
        0x0000FF,
        0xFF00FF,
        0x00FFFF,
        0xFFFFFF,
        0x000000,
        0x000000,
};


static uint32_t ansi_colors_fg_32rgb[] = {
        0x000000,
        0xAA0000,
        0x00AA00,
        0xAAAA00,
        0x0000AA,
        0xAA00AA,
        0x00AAAA,
        0xAAAAAA,
        0x000000,
        0xA9A9A9,
};

static uint32_t ansi_colors_bg_32rgb[] = {
        0x000000,
        0xAA0000,
        0x00AA00,
        0xAAAA00,
        0x0000AA,
        0xAA00AA,
        0x00AAAA,
        0xAAAAAA,
        0x000000,
        0x000000,
};

static uint16_t ansi_colors_bright_bg_16rgb[] = {
        0x10A2,
        0xF800,
        0x07E0,
        0xFFE0,
        0x001F,
        0xF81F,
        0x07FF,
        0xFFFF,
        0x0000,
        0x0000,
};

static uint16_t ansi_colors_bright_fg_16rgb[] = {
        0x10A2,
        0xF800,
        0x07E0,
        0xFFE0,
        0x001F,
        0xF81F,
        0x07FF,
        0xFFFF,
        0x0000,
        0xD69A,
};

static uint16_t ansi_colors_fg_16rgb[] = {
        0x0000,
        0xA000,
        0x0540,
        0xA540,
        0x0014,
        0xA014,
        0x0554,
        0xFFFF,
        0x0000,
        0xA534,
};

static uint16_t ansi_colors_bg_16rgb[] = {
        0x0000,
        0xA000,
        0x0540,
        0xA540,
        0x0014,
        0xA014,
        0x0554,
        0xFFFF,
        0x0000,
        0x0000,
};

static struct virtual_console_driver vc_fbcon_driver = {
        .vc_putc = fbcon_putc,
        .vc_blank = fbcon_blank,
        .vc_get_dimensions = fbcon_get_dimensions,
        .vc_set_attr = fbcon_set_attr,
};

void
fbcon_init(struct fb_info *info)
{
        int i;

        fb_console.fbcon_info = info;
        if ((fb_console.fbcon_font = load_default_font()) == NULL)
                panic("Unable to load bitmap font");
        if (info->mode_info.bpp != 32 && info->mode_info.bpp != 16)
                panic("fbcon_init: Unrecognised bits per pixel");
        for (i = 0; i < NR_VIRTUAL_CONSOLES; i++)
                register_vc_driver(i, &vc_fbcon_driver);
        current_fbcon = &fb_console;
}

static int
fbcon_putc(uint32_t cx, uint32_t cy, int c)
{
        unsigned char *data;
        struct bitmap_font *font;
        struct fb_info *info;
        struct vbe_mode_info *mode_info;
        uint32_t dst, off;

        font = current_fbcon->fbcon_font;
        info = current_fbcon->fbcon_info;
        mode_info = &info->mode_info;
        if (c < 0 || (uint32_t)c >= font->nr_glpyh)
                c = 0;
        data = font->data + c * font->bytes_per_glpyh;
        off = (cy * font->height * mode_info->pitch)
              + (cx * (font->width) * sizeof(uint32_t));
        dst = info->fb_virt_addr + off;
        if (mode_info->bpp == 32)
                fbcon_putc_32bpp(font, data, mode_info->pitch, current_fbcon->fbcon_bg, current_fbcon->fbcon_fg, dst);
        else if (mode_info->bpp == 16)
                fbcon_putc_16bpp(font, data, mode_info->pitch, current_fbcon->fbcon_bg, current_fbcon->fbcon_fg, dst);
        return c;
}

static void
fbcon_putc_32bpp(struct bitmap_font *font, unsigned char *data, uint32_t pitch, uint32_t bg, uint32_t fg, uint32_t dst)
{
        uint32_t mask, bytes_per_line, i, j;

        bytes_per_line = (font->width + 7) >> 3;
        for (i = 0; i < font->height; i++) {
               mask = 1 << (font->width -1);
               for (j = 0; j < font->width; j++) {
                       *(uint32_t *)(dst) = *((uint32_t *)data) & mask ? fg : bg;
                       mask >>= 1;
                       dst += sizeof(uint32_t);
               }
               data += bytes_per_line;
               dst += pitch - font->width * sizeof(uint32_t);
        }
}

static void
fbcon_putc_16bpp(struct bitmap_font *font, unsigned char *data, uint32_t pitch, uint16_t bg, uint16_t fg, uint32_t dst)
{
        uint32_t mask, bytes_per_line, i, j;

        bytes_per_line = (font->width + 7) >> 3;
        for (i = 0; i < font->height; i++) {
               mask = 1 << (font->width -1);
               for (j = 0; j < font->width; j++) {
                       *(uint16_t *)(dst) = *((uint32_t *)data) & mask ? fg : bg;
                       mask >>= 1;
                       dst += sizeof(uint16_t);
               }
               data += bytes_per_line;
               dst += pitch - font->width * sizeof(uint16_t);
        }
}

static void
fbcon_get_dimensions(uint32_t *width, uint32_t *height)
{
        struct fb_info *info;

        info = fb_console.fbcon_info;
        *width = info->mode_info.width / fb_console.fbcon_font->width;
        *height = info->mode_info.height / fb_console.fbcon_font->height;
}

static void
fbcon_blank(uint32_t cx, uint32_t cy)
{
        uint32_t dst, off;
        struct bitmap_font *font;
        struct vbe_mode_info *mode_info;
        struct fb_info *info;

        font = current_fbcon->fbcon_font;
        info = current_fbcon->fbcon_info;
        mode_info = &info->mode_info;
        off = (cy * font->height * mode_info->pitch)
              + (cx * (font->width) * sizeof(uint32_t));
        dst = info->fb_virt_addr + off;
        if (mode_info->bpp == 32)
                fbcon_blank_32bpp(font, mode_info->pitch, dst);
        else if (mode_info->bpp == 16)
                fbcon_blank_16bpp(font, mode_info->pitch, dst);
}

static void
fbcon_blank_32bpp(struct bitmap_font *font, uint32_t pitch, uint32_t dst)
{
        uint32_t i, j;

        for (i = 0; i < font->height; i++) {
                for (j = 0; j < font->width; j++) {
                        *(uint32_t *)(dst) = 0;
                        dst += sizeof(uint32_t);

                }
                dst += pitch - font->width * sizeof(uint32_t);
        }
}

static void
fbcon_blank_16bpp(struct bitmap_font *font, uint32_t pitch, uint32_t dst)
{
        uint32_t i, j;

        for (i = 0; i < font->height; i++) {
                for (j = 0; j < font->width; j++) {
                        *(uint16_t *)(dst) = 0;
                        dst += sizeof(uint16_t);

                }
                dst += pitch - font->width * sizeof(uint16_t);
        }
}

static void
fbcon_set_attr(struct vc_attr *attr, enum vc_attr_type type)
{
        struct fb_info *info;

        info = current_fbcon->fbcon_info;
        if (info->mode_info.bpp == 16)
                fbcon_set_attr_16bpp(attr, type);
        else if (info->mode_info.bpp == 32)
                fbcon_set_attr_32bpp(attr, type);
}

static void
fbcon_set_attr_32bpp(struct vc_attr *attr, enum vc_attr_type type)
{
        if (type == INDEXED_COLOR) {
                if ((attr->fg_indexed >= ANSI_FG_COLOR_BASE && attr->fg_indexed <= ANSI_FG_DEFAULT))
                        current_fbcon->fbcon_fg = ansi_colors_fg_32rgb[attr->fg_indexed - ANSI_FG_COLOR_BASE];
                else if ((attr->fg_indexed >= ANSI_FG_BRIGHT_COLOR_BASE && attr->fg_indexed <= ANSI_FG_BRIGHT_WHITE))
                        current_fbcon->fbcon_fg = ansi_colors_bright_bg_32rgb[attr->fg_indexed - ANSI_FG_BRIGHT_COLOR_BASE];

                if ((attr->bg_indexed >= ANSI_BG_COLOR_BASE && attr->bg_indexed <= ANSI_BG_DEFAULT))
                        current_fbcon->fbcon_bg = ansi_colors_bg_32rgb[attr->bg_indexed - ANSI_BG_COLOR_BASE];
                else if ((attr->bg_indexed >= ANSI_BG_BRIGHT_COLOR_BASE && attr->bg_indexed <= ANSI_BG_BRIGHT_WHITE))
                        current_fbcon->fbcon_bg = ansi_colors_bright_bg_32rgb[attr->bg_indexed - ANSI_BG_BRIGHT_COLOR_BASE];
        } else if (type == RGB) {
                current_fbcon->fbcon_fg = attr->fg_rgb;
                current_fbcon->fbcon_bg = attr->bg_rgb;
        }
        /* For now, ignore all attributes except bold, reverse and dim mode */
        if ((attr->attr & VC_BOLD) && (type == INDEXED_COLOR) && (type <= ANSI_FG_WHITE))
                /* Only works with indexed color */
                current_fbcon->fbcon_fg = ansi_colors_bright_fg_32rgb[attr->fg_indexed - ANSI_FG_COLOR_BASE];
        if ((attr->attr & VC_DIM))
                current_fbcon->fbcon_fg = ansi_colors_bright_fg_32rgb[ANSI_FG_BLACK];
        if ((attr->attr & VC_REVERSE)) {
                uint32_t tmp;

                tmp = current_fbcon->fbcon_fg;
                current_fbcon->fbcon_fg = current_fbcon->fbcon_bg;
                current_fbcon->fbcon_bg = tmp;
        }
}

static void
fbcon_set_attr_16bpp(struct vc_attr *attr, enum vc_attr_type type)
{
        if (type == INDEXED_COLOR) {
                if ((attr->fg_indexed >= ANSI_FG_COLOR_BASE && attr->fg_indexed <= ANSI_FG_DEFAULT))
                        current_fbcon->fbcon_fg = ansi_colors_fg_16rgb[attr->fg_indexed - ANSI_FG_COLOR_BASE];
                else if ((attr->fg_indexed >= ANSI_FG_BRIGHT_COLOR_BASE && attr->fg_indexed <= ANSI_FG_BRIGHT_WHITE))
                        current_fbcon->fbcon_fg = ansi_colors_bright_bg_16rgb[attr->fg_indexed - ANSI_FG_BRIGHT_COLOR_BASE];

                if ((attr->bg_indexed >= ANSI_BG_COLOR_BASE && attr->bg_indexed <= ANSI_BG_DEFAULT))
                        current_fbcon->fbcon_bg = ansi_colors_bg_16rgb[attr->bg_indexed - ANSI_BG_COLOR_BASE];
                else if ((attr->bg_indexed >= ANSI_BG_BRIGHT_COLOR_BASE && attr->bg_indexed <= ANSI_BG_BRIGHT_WHITE))
                        current_fbcon->fbcon_bg = ansi_colors_bright_bg_16rgb[attr->bg_indexed - ANSI_BG_BRIGHT_COLOR_BASE];
        } else if (type == RGB) {
                /* Just check that it is actually 16 bit RGB */
                if (attr->fg_rgb <= (1 << 16) - 1)
                        current_fbcon->fbcon_fg = attr->fg_rgb;
                if (attr->bg_rgb <= (1 << 16) - 1)
                        current_fbcon->fbcon_bg = attr->bg_rgb;
        }
        /* See above comments */
        if ((attr->attr & VC_BOLD) && (type == INDEXED_COLOR) && (type <= ANSI_FG_WHITE))
                current_fbcon->fbcon_fg = ansi_colors_bright_fg_16rgb[attr->fg_indexed - ANSI_FG_COLOR_BASE];
        if ((attr->attr & VC_DIM))
                current_fbcon->fbcon_fg = ansi_colors_bright_fg_16rgb[ANSI_FG_BLACK];
        if ((attr->attr & VC_REVERSE)) {
                uint32_t tmp;

                tmp = current_fbcon->fbcon_fg;
                current_fbcon->fbcon_fg = current_fbcon->fbcon_bg;
                current_fbcon->fbcon_bg = tmp;
        }

}
