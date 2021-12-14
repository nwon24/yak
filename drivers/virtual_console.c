/*
 * virtual_console.c
 * Virtual console driver, built on top of low level console drivers (such as the framebuffer console driver)
 */

#include <drivers/driver.h>
#include <drivers/tty.h>
#include <drivers/virtual_console.h>

static struct virtual_console vc_table[NR_VIRTUAL_CONSOLES];
static struct virtual_console_driver *vc_driver_table[NR_VIRTUAL_CONSOLES];

static int vc_write(int n, struct tty_queue *tq);

static struct tty_driver vc_tty_driver = {
        .driver_out = vc_write,
        .driver_id = _DRIVERS_VIRTUAL_CONSOLE_DRIVER
};

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
                vc->vc_cx = 0;
                vc->vc_cy = 0;
        }
        for (i = 0; i < NR_VIRTUAL_CONSOLES; i++)
                tty_driver_register(i, &vc_tty_driver);
        return 0;
}

static int
vc_write(int n, struct tty_queue *tq)
{
        struct virtual_console *vc;
        struct virtual_console_driver *vcd;
        int nr, i;
        char *p;

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
                } else {
                        vcd->vc_putc(vc, *p);
                        vc->vc_cx++;
                }
                p++;
        }
        return i;
}
