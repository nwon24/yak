/*
 * virtual_console.c
 * Virtual console driver, built on top of low level console drivers (such as the framebuffer console driver)
 * Handles a subset of VT102 escape sequences.
 */

#include <drivers/driver.h>
#include <drivers/tty.h>
#include <drivers/virtual_console.h>

#include <mm/vm.h>

#include <generic/string.h>

#define SCR_POS(vc)	((vc)->vc_cy * (vc)->vc_width + (vc)->vc_cx)

#define VC_XPOS(vc, pos)	((pos) % (vc)->vc_width)
#define VC_YPOS(vc, pos)	((pos) / (vc)->vc_width)

#define NPAR	16

#define TAB_STOP	8

static struct virtual_console vc_table[NR_VIRTUAL_CONSOLES];
static struct virtual_console_driver *vc_driver_table[NR_VIRTUAL_CONSOLES];

static int vc_write(int n, struct tty_queue *tq);
static void vc_flush(struct virtual_console *vc, struct virtual_console_driver *vcd);
static char *vc_start_escape(struct virtual_console *vc, char *p);
static char *vc_csi_escape(struct virtual_console *vc, char *p);

static inline void vc_linefeed(struct virtual_console *vc);
static inline void vc_rlinefeed(struct virtual_console *vc);
static inline void vc_carriage_return(struct virtual_console *vc);
static inline void vc_tab_stop(struct virtual_console *vc);
static inline void vc_save_state(struct virtual_console *vc);
static inline void vc_restore_state(struct virtual_console *vc);

static void vc_calc_last_pos(struct virtual_console *vc);
static void vc_insert_blanks(struct virtual_console *vc, int count);
static void vc_clear_chars(struct virtual_console *vc, int count);
static void vc_insert_lines(struct virtual_console *vc, int count);
static void vc_delete_lines(struct virtual_console *vc, int count);
static void vc_delete_chars(struct virtual_console *vc, int count);
static void vc_clear_screen(struct virtual_console *vc, int par);
static void vc_clear_line(struct virtual_console *vc, int par);
static void vc_scroll_up(struct virtual_console *vc);
static void vc_scroll_down(struct virtual_console *vc);

static struct tty_driver vc_tty_driver = {
        .driver_out = vc_write,
        .driver_id = _DRIVERS_VIRTUAL_CONSOLE_DRIVER
};

static inline void
vc_linefeed(struct virtual_console *vc)
{
        if (vc->vc_cy + 1 < vc->vc_height)
                vc->vc_cy++;
}

static inline void
vc_rlinefeed(struct virtual_console *vc)
{
        if (vc->vc_cy)
                vc->vc_cy--;
}

static inline void
vc_carriage_return(struct virtual_console *vc)
{
        vc->vc_cx = 0;
}

static inline void
vc_tab_stop(struct virtual_console *vc)
{
        if ((vc->vc_cx += TAB_STOP - (vc->vc_cx % TAB_STOP)) > vc->vc_width)
                vc->vc_cx = vc->vc_width;
}

static inline void
vc_save_state(struct virtual_console *vc)
{
        vc->vc_saved_cx = vc->vc_cx;
        vc->vc_saved_cy = vc->vc_cy;
}

static inline void
vc_restore_state(struct virtual_console *vc)
{
        vc->vc_cx = vc->vc_saved_cx;
        vc->vc_cy = vc->vc_saved_cy;
}

void
register_vc_driver(int console, struct virtual_console_driver *driver)
{
        vc_driver_table[console] = driver;
}

int
vc_init(void)
{
        struct virtual_console *vc;
        int i;

        for (vc = vc_table; vc < vc_table + NR_VIRTUAL_CONSOLES; vc++) {
                (*(vc_driver_table + (vc - vc_table)))->vc_get_dimensions(&vc->vc_width, &vc->vc_height);
                vc->vc_scr_size = vc->vc_width * vc->vc_height;
                vc->vc_scr_buf = kvmalloc(vc->vc_scr_size);
                vc->vc_cur_buf = kvmalloc(vc->vc_scr_size);
                vc->vc_last_pos = 0;
                memset(vc->vc_scr_buf, 0, vc->vc_scr_size);
                vc->vc_cx = vc->vc_saved_cx = 0;
                vc->vc_cy = vc->vc_saved_cy = 0;
                vc->vc_scroll_top = 0;
                vc->vc_scroll_bottom = vc->vc_height - 1;
        }
        for (i = 0; i < NR_VIRTUAL_CONSOLES; i++)
                tty_driver_register(i, &vc_tty_driver);
        return 0;
}

static void
vc_flush(struct virtual_console *vc, struct virtual_console_driver *vcd)
{
        char *p1, *p2;

        p1 = vc->vc_scr_buf;
        p2 = vc->vc_cur_buf;
        if (vc->vc_new_last_pos > vc->vc_last_pos)
                vc->vc_last_pos = vc->vc_new_last_pos;
        while ((uint32_t)(p1 - vc->vc_scr_buf) <= vc->vc_last_pos) {
                if (*p1 != *p2) {
                        if (!*p1)
                                vcd->vc_blank(VC_XPOS(vc, p1 - vc->vc_scr_buf), VC_YPOS(vc, p1 - vc->vc_scr_buf));
                        else
                                vcd->vc_putc(VC_XPOS(vc, p1 - vc->vc_scr_buf), VC_YPOS(vc, p1 - vc->vc_scr_buf), *p1);
                        *p2 = *p1;
                }
                p2++;
                p1++;
        }
	if (vc->vc_new_last_pos < vc->vc_last_pos)
                vc->vc_last_pos = vc->vc_new_last_pos;
}

static int
vc_write(int n, struct tty_queue *tq)
{
        struct virtual_console *vc;
        struct virtual_console_driver *vcd;
        int nr, i;
        char *p, *old;

        vc = vc_table + n;
        if (vc >= vc_table + NR_VIRTUAL_CONSOLES)
                return -1;
        vcd = *(vc_driver_table + n);
        nr = tq->tq_tail - tq->tq_head;
        i = nr;
        if (nr < 0)
                return -1;
        p = tq->tq_head;
        while (nr--) {
                if (*p == '\r') {
                        vc->vc_cx = 0;
                } else if (*p == '\n') {
                        vc->vc_cy++;
                } else if (*p == '\b') {
                        if (vc->vc_cx)
                                vc->vc_cx--;
                } else if (*p == '\033') {
                        old = p;
                	p = vc_start_escape(vc, p);
                        nr -= p - old;
                } else {
                        *(vc->vc_scr_buf + SCR_POS(vc)) = *p;
                        vc->vc_cx++;
                }
                p++;
        }
        vc_calc_last_pos(vc);
        vc_flush(vc, vcd);
        return i;
}

static char *
vc_start_escape(struct virtual_console *vc, char *p)
{
        switch (*++p) {
        case 'D':
                vc_linefeed(vc);
                break;
        case 'E':
                vc_linefeed(vc);
                vc_carriage_return(vc);
                break;
        case 'H':
                vc_tab_stop(vc);
                break;
        case 'M':
                vc_rlinefeed(vc);
                break;
        case '7':
                vc_save_state(vc);
                break;
        case '8':
                vc_restore_state(vc);
                break;
        case '[':
                return vc_csi_escape(vc, ++p);
                break;
        default:
                break;
        }
        return p;
}

static char *
vc_csi_escape(struct virtual_console *vc, char *p)
{
        uint32_t par[NPAR], i;

        for (i = 0; i < NPAR; i++)
                par[i] = 0;
        i = 0;
loop:
        if (*p >= '0' && *p <= '9') {
                par[i] = 10 * par[i] + *p - '0';
                p++;
                goto loop;
        }
        if (*p == ';') {
                i++;
                p++;
                goto loop;

        }
        switch (*p) {
        case '@':
                vc_insert_blanks(vc, par[0]);
                return p;
        case 'A':
                if (vc->vc_cy >= par[0])
                        vc->vc_cy -= par[0];
                else
                        vc->vc_cy = 0;
                return p;
        case 'e':
        case 'B':
                if (vc->vc_cy + par[0] < vc->vc_height)
                        vc->vc_cy += par[0];
                else
                        vc->vc_cy = vc->vc_height;
                return p;
        case 'a':
        case 'C':
                if (vc->vc_cx + par[0] < vc->vc_width)
                        vc->vc_cx += par[0];
                else
                        vc->vc_cx = vc->vc_width;
                return p;
        case 'D':
                if (vc->vc_cx >= par[0])
                        vc->vc_cx -= par[0];
                else
                        vc->vc_cx = 0;
                return p;
        case 'E':
                if (vc->vc_cy + par[0] < vc->vc_height)
                        vc->vc_cy += par[0];
                else
                        vc->vc_cy = vc->vc_height;
                vc->vc_cx = 0;
                return p;
        case 'F':
                if (vc->vc_cy >= par[0])
                        vc->vc_cy -= par[0];
                else
                        vc->vc_cy = 0;
                vc->vc_cx = 0;
                return p;
        case 'G':
                if (par[0] < vc->vc_width)
                        vc->vc_cx = par[0];
                return p;
        case 'f':
        case 'H':
                if (par[0] >= 1 && par[0] <= vc->vc_height)
                        vc->vc_cy = par[0] - 1;
                if (par[1] >= 1 && par[1] <= vc->vc_width)
                        vc->vc_cx = par[1] - 1;
                return p;
        case 'J':
                vc_clear_screen(vc, par[0]);
                return p;
        case 'K':
                vc_clear_line(vc, par[0]);
                return p;
        case 'L':
                vc_insert_lines(vc, par[0]);
                return p;
        case 'M':
                vc_delete_lines(vc, par[0]);
                return p;
        case 'P':
                vc_delete_chars(vc, par[0]);
                return p;
        case 'X':
                vc_clear_chars(vc, par[0]);
                return p;
        case 'd':
                if (par[0] < vc->vc_height)
                        vc->vc_cy = par[0];
                return p;
        default:
                return p;
        }
}

static void
vc_insert_blanks(struct virtual_console *vc, int count)
{
        char *p;

        p = vc->vc_scr_buf + SCR_POS(vc);
        if (vc->vc_cx + count >= vc->vc_width) {
                memset(p, 0, vc->vc_width - vc->vc_cx);
        } else {
                memmove(p + count, p, vc->vc_width - vc->vc_cx - count);
                memset(p, 0, count);
        }
}

static void
vc_clear_chars(struct virtual_console *vc, int count)
{
        char *p;

        p = vc->vc_scr_buf + SCR_POS(vc);
        if ((uint32_t)count > vc->vc_width - vc->vc_cx)
                count = vc->vc_width - vc->vc_cx;
        while (count--)
                *p++ = 0;
}

static void
vc_delete_chars(struct virtual_console *vc, int count)
{
        char *p;

        p = vc->vc_scr_buf + SCR_POS(vc);
        if ((uint32_t)count > vc->vc_width - vc->vc_cx)
                count = vc->vc_width - vc->vc_cx;
        memmove(p, p + count, vc->vc_width - vc->vc_cx - count);
        memset(p + count, 0, vc->vc_width - vc->vc_cx - count);
}

static void
vc_clear_screen(struct virtual_console *vc, int par)
{
        char *start;
        uint32_t count;
        int pos = SCR_POS(vc);

        switch (par) {
        case 0: /* Clear from cursor to end of screen */
                start = vc->vc_scr_buf + pos;
                count = vc->vc_scr_size - pos;
                break;
        case 1: /* Clear from cursor to beginning of screen */
                start = vc->vc_scr_buf;
                count = pos;
                break;
        case 2: /* Clear entire screen */
                start = vc->vc_scr_buf;
                count = vc->vc_scr_size;
                break;
        default:
                return;
        }
        memset(start, 0, count);
}

static void
vc_clear_line(struct virtual_console *vc, int par)
{
        char *start;
        uint32_t count;
        int pos = SCR_POS(vc);

        switch (par) {
        case 0: /* Clear from cursor to end of line */
                start = vc->vc_scr_buf + pos;
                count = vc->vc_width - vc->vc_cx;
                break;
        case 1: /* Clear from cursor to start of line */
                start = vc->vc_scr_buf + pos - vc->vc_cx;
                count = vc->vc_cx;
                break;
        case 2: /* Clear entire line */
                start = vc->vc_scr_buf + pos - vc->vc_cx;
                count = vc->vc_width;
                break;
        default:
                return;
        }
        memset(start, 0, count);
}

static void
vc_calc_last_pos(struct virtual_console *vc)
{
        char *p;

        vc->vc_new_last_pos = 0;
        for (p = vc->vc_scr_buf; p < vc->vc_scr_buf + vc->vc_scr_size; p++) {
                if (*p)
                        vc->vc_new_last_pos = p - vc->vc_scr_buf;
        }
}

static void
vc_scroll_up(struct virtual_console *vc)
{
        char *dst, *src;
        int count;

        dst = vc->vc_scr_buf + vc->vc_scroll_top * vc->vc_width;
        src = dst + vc->vc_width;
        count = (vc->vc_scroll_bottom - vc->vc_scroll_top) * vc->vc_width;
        memmove(dst, src, count);
        memset(vc->vc_scr_buf + vc->vc_scroll_bottom * vc->vc_width, 0, vc->vc_width);
}

static void
vc_scroll_down(struct virtual_console *vc)
{
        char *dst, *src;
        int count;

        dst = vc->vc_scr_buf + vc->vc_scroll_bottom * vc->vc_width + vc->vc_width - 1;
        src = dst - vc->vc_width;
        count = (vc->vc_scroll_bottom - vc->vc_scroll_top) * vc->vc_width;
        /* We need to copy backwards so memmove() cannot be used. */
        while (count--)
                *dst-- = *src--;
        memset(vc->vc_scr_buf + vc->vc_scroll_top * vc->vc_width, 0, vc->vc_width);
}

static void
vc_insert_lines(struct virtual_console *vc, int count)
{
        uint32_t old_scroll_top, old_scroll_bottom;

        old_scroll_top = vc->vc_scroll_top;
        old_scroll_bottom = vc->vc_scroll_bottom;
        vc->vc_scroll_top = vc->vc_cy;
        vc->vc_scroll_bottom = vc->vc_height - 1;
        while (count--)
                vc_scroll_down(vc);
        vc->vc_scroll_top = old_scroll_top;
        vc->vc_scroll_bottom = old_scroll_bottom;
}

static void
vc_delete_lines(struct virtual_console *vc, int count)
{
        uint32_t old_scroll_top, old_scroll_bottom;

        old_scroll_top = vc->vc_scroll_top;
        old_scroll_bottom = vc->vc_scroll_bottom;
        vc->vc_scroll_top = vc->vc_cy;
        vc->vc_scroll_bottom = vc->vc_height - 1;
        while (count--)
                vc_scroll_up(vc);
        vc->vc_scroll_top = old_scroll_top;
        vc->vc_scroll_bottom = old_scroll_bottom;
}
