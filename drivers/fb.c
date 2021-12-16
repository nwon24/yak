/*
 * fb.c
 * Generic framebuffer driver.
 */

#include <asm/paging.h>

#include <drivers/fb.h>
#include <drivers/fbcon.h>

#include <kernel/debug.h>

#include <generic/multiboot.h>

struct fb_info current_fb_info;

#define warn_no_fb()	printk("WARNING: No framebuffer")

int
fb_init(void)
{
        struct vbe_mode_info *vbe_mode;

        if (!(__mb_info.flags & MULTIBOOT_INFO_VBE_INFO)) {
                printk("WARNING: Bootloader has not provided VBE info");
                warn_no_fb();
                return -1;
        }
        vbe_mode = (struct vbe_mode_info *)VIRT_ADDR(__mb_info.vbe_mode_info);
        if (__mb_info.framebuffer_height != vbe_mode->height ||
            __mb_info.framebuffer_width != vbe_mode->width ||
            __mb_info.framebuffer_bpp != vbe_mode->bpp) {
                printk("WARNING: Inconsistent multiboot framebuffer fields and VBE mode info fields");
                warn_no_fb();
                return -1;
        }
        current_fb_info.mode_info = *vbe_mode;
        if (vbe_mode->bpp != 32 && vbe_mode->bpp != 16) {
                printk("bpp %d\r\n", current_fb_info.mode_info.bpp);
                panic("WARNING: Anything less than 32bpp not supported\r\n");
                warn_no_fb();
        }
        current_fb_info.fb_virt_addr = virt_map_phys(__mb_info.framebuffer_addr_low);
        fbcon_init(&current_fb_info);
        return 0;
}
