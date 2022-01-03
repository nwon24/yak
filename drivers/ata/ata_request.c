/*
 * ata_request.c
 * Handles requests to the ATA controller.
 */
#include <stddef.h>

#include <kernel/config.h>
#include <kernel/debug.h>
#include <kernel/proc.h>

#include <drivers/ata.h>

static void ata_start_request_lba28(struct ata_request *req);
static void ata_start_request_lba48(struct ata_request *req);

struct ata_request ata_requests[ATA_MAX_REQUESTS];

struct ata_request *ata_current_req = NULL;

struct ata_request *
ata_build_request(struct ata_device *dev, size_t lba, size_t count, int cmd, char *buf)
{
	struct ata_request *req;

	for (req = ata_requests; req < ata_requests + ATA_MAX_REQUESTS; req++) {
		if (req->dev == NULL)
			break;
	}
	if (req >= ata_requests + ATA_MAX_REQUESTS)
		return NULL;
	req->dev = dev;
	req->lba = lba;
	req->count = count;
	req->cmd = cmd;
	req->buf = buf;
	req->error = 0;
	req->next = NULL;
	return req;
}

/*
 * Returns 1 if there are no other requests, indicating
 * to the caller that it must begin doing the request
 * immediately.
 */
int
ata_add_request(struct ata_request *req)
{
	struct ata_request *tmp;

	if (ata_current_req == NULL) {
		ata_current_req = req;
		return 1;
	} else {
		for (tmp = ata_current_req; tmp->next != NULL; tmp = tmp->next);
		tmp->next = req;
		return 0;
	}
}

void
ata_finish_request(struct ata_request *req)
{
	req->dev = NULL;
}

void
ata_start_request(struct ata_request *req)
{
	if (req->lba <= 0x0FFFFFFF)
		ata_start_request_lba28(req);
	else
		ata_start_request_lba48(req);
}


/*
 * With these next two routines, we are assuming the caller knows what they are doing,
 * i.e., 48-bit LBA requires the ATA_CMD_READ_SECTORS_EXT and ATA_CMD_WRITE_SECTORS_EXT commands.
 */
static void
ata_start_request_lba28(struct ata_request *req)
{
	struct ata_device *dev = req->dev;

	ata_select_drive(req->dev, 1, req->lba);
	ata_reg_write(dev, (unsigned char)req->count, ATA_REG_SEC_COUNT);
	ata_reg_write(dev, req->lba & 0xFF, ATA_REG_LBA1);
	ata_reg_write(dev, (req->lba >> 8) & 0xFF, ATA_REG_LBA2);
	ata_reg_write(dev, (req->lba >> 16) & 0xFF, ATA_REG_LBA3);
	ata_reg_write(dev, req->cmd, ATA_REG_CMD);
}

#ifdef CONFIG_ARCH_X86
static void
ata_start_request_lba48(struct ata_request *req)
{
	struct ata_device *dev = req->dev;

	/*
	 * All bits are ignored except for LBA bit and drive bit when selected for 48-bit LBA addressing.
	 * All bytes of LBA are sent to LBA registers.
	 */
	ata_select_drive(req->dev, 0, 0);
	ata_reg_write(dev, (req->count >> 8) & 0xFF, ATA_REG_SEC_COUNT);
	/*
	 * Numbering the 48-bit LBA value from low to high, bytes 5 and 6 are zero if we are on
	 * a 32-bit system. This is why we have the '#ifdef..." above. If we try shifting a 32-bit value
	 * more than 32-bits to the left to get the upper 16 bits, the compiler won't be happy.
	 */
	ata_reg_write(dev, (req->lba >> 24) & 0xFF, ATA_REG_LBA1);
	ata_reg_write(dev, 0, ATA_REG_LBA2);
	ata_reg_write(dev, 0, ATA_REG_LBA3);
	ata_reg_write(dev, req->count & 0xFF, ATA_REG_SEC_COUNT);
	ata_reg_write(dev, req->lba & 0xFF, ATA_REG_LBA1);
	ata_reg_write(dev, (req->lba >> 8) & 0xFF, ATA_REG_LBA2);
	ata_reg_write(dev, (req->lba >> 16) & 0xFF, ATA_REG_LBA3);
	ata_reg_write(dev, req->cmd, ATA_REG_CMD);
}
#endif /* CONFIG_ARCH_X86 */
