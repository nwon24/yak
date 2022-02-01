#ifndef KEYBOARD_H
#define KEYBOARD_H

#define CTRL	(1 << 0)
#define ALT	(1 << 1)
#define SHIFT	(1 << 2)

#define MAKE	1
#define BREAK	2

struct kbd_packet {
	uint16_t keycode;
	int type;
};

void kbd_handle_packet(struct kbd_packet *packet);

#endif
