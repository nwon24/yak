/*
 * fbcon.c
 * Framebuffer console driver.
 */

#include <stddef.h>

#include <drivers/ansi_vt.h>
#include <drivers/fb.h>
#include <drivers/fbcon.h>
#include <drivers/vbe.h>

#include <kernel/debug.h>

static struct fbcon fb_console;
static struct fbcon *current_fbcon;

static int fbcon_putc(int c);
static void fbcon_putc_32bpp(struct bitmap_font *font, unsigned char *data, uint32_t pitch, uint32_t bg, uint32_t fg, uint32_t dst);
static void fbcon_putc_16bpp(struct bitmap_font *font, unsigned char *data, uint32_t pitch, uint16_t bg, uint16_t fg, uint32_t dst);

static uint32_t ansi_colors_fg_32rgb[] = {
        0x000000,	/* Black */
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

static uint32_t ansi_colors_bg_32rgb[] = {
        0x000000,
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

static uint16_t ansi_colors_bg_16rgb[] = {
        0x0000,
        0xF800,
        0x07E0,
        0x001F,
        0xF81F,
        0x07FF,
        0xFFFF,
        0x0000,
        0x0000,
};

static uint16_t ansi_colors_fg_16rgb[] = {
        0x0000,
        0xF800,
        0x07E0,
        0x001F,
        0xF81F,
        0x07FF,
        0xFFFF,
        0x0000,
        0xD69A,
};

void
fbcon_init(struct fb_info *info)
{
        fb_console.fbcon_info = info;
        printk("bpp %d, height %d width %d\r\n", info->mode_info.bpp, info->mode_info.height, info->mode_info.width);
        if ((fb_console.fbcon_font = load_default_font()) == NULL)
                panic("Unable to load bitmap font");
        fb_console.fbcon_cx = 0;
        fb_console.fbcon_cy = 0;
        if (info->mode_info.bpp == 32) {
                fb_console.fbcon_fg = ansi_colors_fg_32rgb[ANSI_FG_DEFAULT - ANSI_FG_COLOR_BASE];
                fb_console.fbcon_bg = ansi_colors_bg_32rgb[ANSI_BG_DEFAULT - ANSI_BG_COLOR_BASE];
        } else if (info->mode_info.bpp == 16) {
                fb_console.fbcon_fg = ansi_colors_fg_16rgb[ANSI_FG_DEFAULT - ANSI_FG_COLOR_BASE];
                fb_console.fbcon_bg = ansi_colors_bg_16rgb[ANSI_BG_DEFAULT - ANSI_BG_COLOR_BASE];
        } else {
                panic("fbcon_init: Unrecognised bits per pixel");
        }
        current_fbcon = &fb_console;
}

int
fbcon_puts(char *s, int len)
{
        char *p;

        p = s;
        while (len-- && *s)
                fbcon_putc(*s++);
        return p - s;
}

static int
fbcon_putc(int c)
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
        off = (current_fbcon->fbcon_cy * font->height * mode_info->pitch)
              + (current_fbcon->fbcon_cx * (font->width + 1) * sizeof(uint32_t));
        dst = info->fb_virt_addr + off;
        if (mode_info->bpp == 32)
                fbcon_putc_32bpp(font, data, mode_info->pitch, current_fbcon->fbcon_bg, current_fbcon->fbcon_fg, dst);
        else if (mode_info->bpp == 16)
                fbcon_putc_16bpp(font, data, mode_info->pitch, current_fbcon->fbcon_bg, current_fbcon->fbcon_fg, dst);
        current_fbcon->fbcon_cx++;
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
