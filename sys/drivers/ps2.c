/*
 * ps2.c
 * PS/2 Controller driver.
 */
#include <stdarg.h>

#include <asm/port_io.h>

#include <drivers/ps2.h>

#include <kernel/debug.h>

static int dual_channel;

static int
ps2_send_cmd(int cmd, int arg, int ret, ...)
{
	va_list args;

	va_start(args, ret);
	outb(cmd, PS2_CMD_REG);
	if (arg) {
		while (inb(PS2_STAT_REG) & PS2_STAT_IN_FULL);
		outb(va_arg(args, int), PS2_DATA_PORT);
	}
	va_end(args);
	if (ret) {
		while (!(inb(PS2_STAT_REG) & PS2_STAT_OUT_FULL));
		return inb(PS2_DATA_PORT);
	}
	return 0;
}

int
ps2_dev_send_wait(enum ps2_channel chan, int data)
{
	int retries, timeout, res;

	retries = 3;
	timeout = 10000;
	while (retries--) {
		ps2_dev_send_data(chan, data);
		while (!(inb(PS2_STAT_REG) & PS2_STAT_OUT_FULL)
		       && timeout--);
		if (timeout == 0)
			return -1;
		if ((res = inb(PS2_DATA_PORT)) == PS2_DEV_ACK)
			break;
		if (res == PS2_DEV_RESEND)
			continue;
		return -1;
	}
	return 0;
}

int
ps2_dev_send_data(enum ps2_channel chan, int data)
{
	int timeout = 10000;

	switch (chan) {
	case PS2_FIRST_PORT:
		while ((inb(PS2_STAT_REG) & PS2_STAT_IN_FULL)
		       && timeout--);
		if (timeout == 0)
			return -1;
		outb(data, PS2_DATA_PORT);
		break;
	case PS2_SEC_PORT:
		if (!dual_channel)
			return -1;
		ps2_send_cmd(PS2_CMD_WR_SEC_IN, 0, 0);
		while ((inb(PS2_STAT_REG) & PS2_STAT_IN_FULL)
		       && timeout--);
		if (timeout == 0)
			return -1;
		outb(data, PS2_DATA_PORT);
		break;
	}
	return 0;
}

void
ps2_init(void)
{
	int config_byte, ret, working_devs, test;

	/* Disable devices */
	ps2_send_cmd(PS2_CMD_DISABLE_FIRST_PORT, 0, 0);
	ps2_send_cmd(PS2_CMD_DISABLE_SEC_PORT, 0, 0);
	/* Flush output buffer */
	inb(PS2_DATA_PORT);
	config_byte = ps2_send_cmd(PS2_CMD_RD_BYTE0, 0, 1);
	config_byte &= ~PS2_FIRST_PORT_INTR;
	config_byte &= ~PS2_SEC_PORT_INTR;
	config_byte &= ~PS2_FIRST_TRANSLATION;
	if (config_byte & PS2_SECOND_CLOCK)
		dual_channel = 1;
	ps2_send_cmd(PS2_CMD_WR_BYTE0, 1, 0, config_byte);
	ret = ps2_send_cmd(PS2_CMD_TEST, 0, 1);
	if (ret != 0x55)
		panic("Unable to initialise PS/2 controller");
	if (dual_channel) {
		ps2_send_cmd(PS2_CMD_ENABLE_SEC_PORT, 0, 0);
		config_byte = ps2_send_cmd(PS2_CMD_RD_BYTE0, 0, 1);
		if (config_byte & PS2_SECOND_CLOCK)
			dual_channel = 0;
		else
			ps2_send_cmd(PS2_CMD_DISABLE_SEC_PORT, 0, 0);
	}
	working_devs = 0;
	test = ps2_send_cmd(PS2_CMD_TEST_FIRST_PORT, 0, 1);
	if (test == 0)
		working_devs++;
	if (dual_channel) {
		test = ps2_send_cmd(PS2_CMD_TEST_SEC_PORT, 0, 1);
		if (test == 0)
			working_devs++;
	}
	if (working_devs == 0)
		panic("No working devices connected to PS/2 controller");
	ps2_send_cmd(PS2_CMD_ENABLE_FIRST_PORT, 0, 0);
	config_byte = ps2_send_cmd(PS2_CMD_RD_BYTE0, 0, 1);
	config_byte |= PS2_FIRST_PORT_INTR;
	config_byte |= PS2_FIRST_TRANSLATION;
	if (working_devs == 2) {
		ps2_send_cmd(PS2_CMD_ENABLE_SEC_PORT, 0, 0);
		config_byte |= PS2_SEC_PORT_INTR;
	}
	ps2_send_cmd(PS2_CMD_WR_BYTE0, 1, 0, config_byte);
}
