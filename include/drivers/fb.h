#ifndef _FB_H
#define _FB_H

#include <stdint.h>

#include <drivers/vbe.h>

struct fb_info {
        struct vbe_mode_info mode_info;
        uint32_t fb_virt_addr;
};

int fb_init(void);

#endif /* _FB_H */
