/*
 * Didn't feel write to call it hd_* something
 * as SSDs are not disks...
 */
#ifndef DRIVE_H
#define DRIVE_H

#include <stddef.h>

#include <kernel/config.h>

#ifdef CONFIG_DISK_MBR
#include <drivers/mbr.h>
#else
#error "We don't support GPT yet."
#endif /* CONFIG_DISK_MBR */

#define SECTOR_SIZE	512

struct generic_disk_dev {
	struct drive_driver *driver;
	int minor;
	size_t lba_start;
	size_t lba_len;
};

struct drive_driver {
	int (*drive_start)(int drive, char *buf, size_t count, size_t lba, int rw);
	int (*drive_exists)(int dev);
	void (*drive_intr)(void);
};

int drive_init(void);
int drive_rw(int minor, size_t block_off, char *buf, size_t count, int rw);
void drive_driver_register(struct drive_driver *drv);

#endif /* DRIVE_H */
