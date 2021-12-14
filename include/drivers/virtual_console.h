#ifndef _VIRTUAL_CONSOLE_H
#define _VIRTUAL_CONSOLE_H

#include <stdint.h>

/* I think Linux has 63... */
#define NR_VIRTUAL_CONSOLES	4

struct virtual_console {
        uint32_t vc_cx;
        uint32_t vc_cy;
        uint32_t vc_width;
        uint32_t vc_height;
};

struct virtual_console_driver {
        int (*vc_putc)(struct virtual_console *vc, int c);
        int (*vc_puts)(struct virtual_console *vc, char *s, int len);
        void (*vc_get_dimensions)(uint32_t *width, uint32_t *height);
};

int vc_init(void);
void register_vc_driver(int console, struct virtual_console_driver *driver);

#endif /* _VIRTUAL_CONSOLE_H */
