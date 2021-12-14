#ifndef _VIRTUAL_CONSOLE_H
#define _VIRTUAL_CONSOLE_H

#include <stdint.h>

/* I think Linux has 63... */
#define NR_VIRTUAL_CONSOLES	4

/*
 * Structure of a virtual console.
 * @vc_cx: Current virtual x coordinate after a write.
 * @vc_cy: Current virtual y coordinate after a wrie
 * @vc_old_cx: Value of x coorindate before write
 * @vc_old_cy: Value of y coordinate before write
 * After vc_flush(), all the above variables should be the same.
 * @vc_width: Width of console in characters (fbcon works this out)
 * @vc_height: Same, but for height
 * @vc_scr_buf: Internal buffer of characters written to the screen.
 * This is used because it makes it easier to do scrolling, clearing, etc.
 * @vc_scr_size: Size of internal buffer in bytes.
 */
struct virtual_console {
        uint32_t vc_cx;
        uint32_t vc_cy;
        uint32_t vc_old_cx;
        uint32_t vc_old_cy;
        uint32_t vc_width;
        uint32_t vc_height;
        char *vc_scr_buf;
        uint32_t vc_scr_size;
};

struct virtual_console_driver {
        int (*vc_putc)(struct virtual_console *vc, int c);
        int (*vc_puts)(struct virtual_console *vc, char *s, int len);
        void (*vc_get_dimensions)(uint32_t *width, uint32_t *height);
};

int vc_init(void);
void register_vc_driver(int console, struct virtual_console_driver *driver);

#endif /* _VIRTUAL_CONSOLE_H */
