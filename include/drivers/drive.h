/*
 * Didn't feel write to call it hd_* something
 * as SSDs are not disks...
 */
#ifndef DRIVE_H
#define DRIVE_H

#include <stddef.h>

#define SECTOR_SIZE	512

struct drive_driver {
	int (*drive_start)(int drive, char *buf, size_t count, size_t lba, int rw);
	void (*drive_intr)(void);
};

int drive_init(void);
void drive_driver_register(struct drive_driver *drv);

#endif /* DRIVE_H */
