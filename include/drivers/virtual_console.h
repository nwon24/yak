#ifndef _VIRTUAL_CONSOLE_H
#define _VIRTUAL_CONSOLE_H

#include <stdint.h>

/* I think Linux has 63... */
#define NR_VIRTUAL_CONSOLES	4

#define VC_BOLD	(1 << 0)
#define VC_DIM		(1 << 1)
#define VC_ITALIC	(1 << 2)
#define VC_UNDERLINE	(1 << 4)
#define VC_BLINKING	(1 << 5)
#define VC_REVERSE	(1 << 6)
#define VC_HIDDEN	(1 << 7)
#define VC_STRIKETHROUGH	(1 << 8)

enum vc_attr_type {
        NONE,		/* Do not set any colour */
        RGB,
        INDEXED_COLOR
};

struct vc_attr {
        uint32_t attr;		/* VC_* attributes above */
        uint32_t fg_rgb;	/* Set RGB directly */
        uint32_t bg_rgb;	/* "" */
        uint32_t fg_indexed;	/* 16 colour pallete index (see ANSI escape sequences) */
        uint32_t bg_indexed;	/* "" */
};

/*
 * Structure of a virtual console.
 * @vc_cx: Current virtual x coordinate after a write.
 * @vc_cy: Current virtual y coordinate after a wrie
 * @vc_last_pos: Final position in our buffer where a character is actually stored.
 * @vc_next_last_pos: Next value of vc_last_pos after we have flushed the current buffer to the screen.
 * @vc_width: Width of console in characters (fbcon works this out)
 * @vc_height: Same, but for height
 * @vc_scr_buf: Internal buffer of characters to be written to the screen.
 * @vc_cur_buf: Internal buffer of characters currently being displayed.
 * This is used because it makes it easier to do scrolling, clearing, etc.
 * @vc_scr_size: Size of internal buffer in bytes.
 * @vc_saved_* is for the ESC 7/8 escape sequence.
 * @vc_scroll_* are the scroll parameters.
 */
struct virtual_console {
        uint32_t vc_cx;
        uint32_t vc_cy;

        uint32_t vc_saved_cx;
        uint32_t vc_saved_cy;

        uint32_t vc_last_pos;
        uint32_t vc_new_last_pos;

        uint32_t vc_width;
        uint32_t vc_height;
        char *vc_scr_buf;
        char *vc_cur_buf;
        uint32_t vc_scr_size;

        uint32_t vc_scroll_top;
        uint32_t vc_scroll_bottom;

        struct vc_attr vc_attr;
};

struct virtual_console_driver {
        int (*vc_putc)(uint32_t cx, uint32_t cy, int c);
        void (*vc_blank)(uint32_t cx, uint32_t cy);
        void (*vc_get_dimensions)(uint32_t *width, uint32_t *height);
        void (*vc_set_attr)(struct vc_attr *attr, enum vc_attr_type type);
};

int vc_init(void);
void register_vc_driver(int console, struct virtual_console_driver *driver);

#endif /* _VIRTUAL_CONSOLE_H */
