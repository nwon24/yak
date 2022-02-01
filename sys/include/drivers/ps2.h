#ifndef PS2_H
#define PS2_H

#define PS2_DATA_PORT	0x60
#define PS2_STAT_REG	0x64
#define PS2_CMD_REG	0x64

#define PS2_STAT_OUT_FULL	(1 << 0)
#define PS2_STAT_IN_FULL	(1 << 1)
#define PS2_STAT_SYS_FLAG	(1 << 2)
#define PS2_STAT_DATA_FOR_PS2	(1 << 3)
#define PS2_STAT_UNUSED1	(1 << 4)
#define PS2_STAT_UNUSED2	(1 << 5)
#define PS2_STAT_TIMEOUT_ERR	(1 << 6)
#define PS2_STAT_PARITY_ERR	(1 << 7)

#define PS2_CMD_RD_BYTE0	0x20
#define PS2_CMD_WR_BYTE0	0x60
#define PS2_CMD_DISABLE_SEC_PORT	0xA7
#define PS2_CMD_ENABLE_SEC_PORT	0xA8
#define PS2_CMD_TEST_SEC_PORT	0xA9
#define PS2_CMD_TEST		0xAA
#define PS2_CMD_TEST_FIRST_PORT	0xAB
#define PS2_CMD_DIAG_DUMP	0xAC
#define PS2_CMD_DISABLE_FIRST_PORT	0xAD
#define PS2_CMD_ENABLE_FIRST_PORT	0xAE
#define PS2_CMD_RD_CONTRL_IN	0xC0
#define PS2_CMD_CP_0_TO_3	0xC1
#define PS2_CMD_CP_4_TO_7	0xC2
#define PS2_CMD_READ_CONTRL_OUT	0xD0
#define PS2_CMD_WR_CONTRL_OUT	0xD1
#define PS2_CMD_WR_FIRST_OUT	0xD2
#define PS2_CMD_WR_SEC_OUT	0xD3
#define PS2_CMD_WR_SEC_IN	0xD4

#define PS2_FIRST_PORT_INTR	(1 << 0)
#define PS2_SEC_PORT_INTR	(1 << 1)
#define PS2_PASSED_POST		(1 << 2)
#define PS2_FIRST_CLOCK		(1 << 4)
#define PS2_SECOND_CLOCK	(1 << 5)
#define PS2_FIRST_TRANSLATION	(1 << 6)

#define PS2_A20_GATE		(1 << 1)
#define PS2_SEC_PORT_CLOCK	(1 << 2)
#define PS2_SEC_PORT_DATA	(1 << 3)
#define PS2_OUT_FULL_FIRST	(1 << 4)
#define PS2_OUT_FULL_SEC	(1 << 5)
#define PS2_FIRST_PORT_CLOCK	(1 << 6)
#define PS2_FIRST_PORT_DATA	(1 << 7)

#define PS2_DEV_ACK		0xFA
#define PS2_DEV_RESEND		0xFE
#define PS2_DEV_CMD_DISABLE_SCAN	0xF5
#define PS2_DEV_CMD_ENABLE_SCAN	0xF4
#define PS2_DEV_CMD_IDENTIFY	0xF2
#define PS2_DEV_CMD_RESET	0xFF

#define PS2_FIRST_IRQ	0x21
#define PS2_SEC_IRQ	0x2C

enum ps2_channel {
	PS2_FIRST_PORT,
	PS2_SEC_PORT,
};

enum ps2_dev_types {
	PS2_MOUSE = 0x0,
	PS2_MOUSE_WITH_SCROLL = 0x3,
	PS2_5_BUTTON_MOUSE = 0x4,
	PS2_MF2_KBD = 0xAB
};

int ps2_dev_send_data(enum ps2_channel chan, int data);
int ps2_dev_send_wait(enum ps2_channel chan, int data);
void ps2_init(void);
int ps2_kbd_init(void);

#endif
