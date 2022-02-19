#ifndef DEFKEYMAP_H
#define DEFKEYMAP_H

#include <stdint.h>

#include <drivers/keycode.h>

static const uint8_t defkeymap[] = {
	[KEY_RAW_1] = '1',
	[KEY_RAW_2] = '2',
	[KEY_RAW_3] = '3',
	[KEY_RAW_4] = '4',
	[KEY_RAW_5] = '5',
	[KEY_RAW_6] = '6',
	[KEY_RAW_7] = '7',
	[KEY_RAW_8] = '8',
	[KEY_RAW_9] = '9',
	[KEY_RAW_0] = '0',
	[KEY_RAW_BACK_TICK] = '`',
	[KEY_RAW_MINUS] = '-',
	[KEY_RAW_EQUALS] = '=',
	[KEY_RAW_BACKSPACE] = '\b',
	[KEY_RAW_TAB] = '\t',
	[KEY_RAW_A] = 'a',
	[KEY_RAW_B] = 'b',
	[KEY_RAW_C] = 'c',
	[KEY_RAW_D] = 'd',
	[KEY_RAW_E] = 'e',
	[KEY_RAW_F] = 'f',
	[KEY_RAW_G] = 'g',
	[KEY_RAW_H] = 'h',
	[KEY_RAW_I] = 'i',
	[KEY_RAW_J] = 'j',
	[KEY_RAW_K] = 'k',
	[KEY_RAW_L] = 'l',
	[KEY_RAW_M] = 'm',
	[KEY_RAW_N] = 'n',
	[KEY_RAW_O] = 'o',
	[KEY_RAW_P] = 'p',
	[KEY_RAW_Q] = 'q',
	[KEY_RAW_R] = 'r',
	[KEY_RAW_S] = 's',
	[KEY_RAW_T] = 't',
	[KEY_RAW_U] = 'u',
	[KEY_RAW_V] = 'v',
	[KEY_RAW_W] = 'w',
	[KEY_RAW_X] = 'x',
	[KEY_RAW_Y] = 'y',
	[KEY_RAW_Z] = 'z',
	[KEY_RAW_LSQR_BRACKET] = '[',
	[KEY_RAW_RSQR_BRACKET] = ']',
	[KEY_RAW_SEMI_COLON] = ';',
	[KEY_RAW_QUOTE] = '\'',
	[KEY_RAW_ENTER] = '\n',
	[KEY_RAW_COMMA] = ',',
	[KEY_RAW_DOT] = '.',
	[KEY_RAW_SLASH] = '/',
	[KEY_RAW_SPACE] = ' ', 
	[KEY_RAW_PAD_1] = '1',
	[KEY_RAW_PAD_2] = '2',
	[KEY_RAW_PAD_3] = '3',
	[KEY_RAW_PAD_4] = '4',
	[KEY_RAW_PAD_5] = '5',
	[KEY_RAW_PAD_6] = '6',
	[KEY_RAW_PAD_7] = '7',
	[KEY_RAW_PAD_8] = '8',
	[KEY_RAW_PAD_9] = '9',
	[KEY_RAW_PAD_0] = '0',
	[KEY_RAW_PAD_SLASH] = '/',
	[KEY_RAW_PAD_MINUS] = '-',
	[KEY_RAW_PAD_STAR] = '*',
	[KEY_RAW_PAD_PLUS] = '+',
	[KEY_RAW_PAD_ENTER] = '\n',
	[KEY_RAW_PAD_DOT] = '.',
};

static const uint8_t defkeymap_shift[] = {
	[KEY_RAW_1] = '!',
	[KEY_RAW_2] = '@',
	[KEY_RAW_3] = '#',
	[KEY_RAW_4] = '$',
	[KEY_RAW_5] = '%',
	[KEY_RAW_6] = '^',
	[KEY_RAW_7] = '&',
	[KEY_RAW_8] = '*',
	[KEY_RAW_9] = '(',
	[KEY_RAW_0] = ')',
	[KEY_RAW_BACK_TICK] = '~',
	[KEY_RAW_MINUS] = '_',
	[KEY_RAW_EQUALS] = '+',
	[KEY_RAW_BACKSPACE] = '\b',
	[KEY_RAW_TAB] = '\t',
	[KEY_RAW_A] = 'A',
	[KEY_RAW_B] = 'B',
	[KEY_RAW_C] = 'C',
	[KEY_RAW_D] = 'D',
	[KEY_RAW_E] = 'E',
	[KEY_RAW_F] = 'F',
	[KEY_RAW_G] = 'G',
	[KEY_RAW_H] = 'H',
	[KEY_RAW_I] = 'I',
	[KEY_RAW_J] = 'J',
	[KEY_RAW_K] = 'K',
	[KEY_RAW_L] = 'L',
	[KEY_RAW_M] = 'M',
	[KEY_RAW_N] = 'N',
	[KEY_RAW_O] = 'O',
	[KEY_RAW_P] = 'P',
	[KEY_RAW_Q] = 'Q',
	[KEY_RAW_R] = 'R',
	[KEY_RAW_S] = 'S',
	[KEY_RAW_T] = 'T',
	[KEY_RAW_U] = 'U',
	[KEY_RAW_V] = 'V',
	[KEY_RAW_W] = 'W',
	[KEY_RAW_X] = 'X',
	[KEY_RAW_Y] = 'Y',
	[KEY_RAW_Z] = 'Z',
	[KEY_RAW_LSQR_BRACKET] = '{',
	[KEY_RAW_RSQR_BRACKET] = '}',
	[KEY_RAW_SEMI_COLON] = ':',
	[KEY_RAW_QUOTE] = '\"',
	[KEY_RAW_ENTER] = '\n',
	[KEY_RAW_COMMA] = '<',
	[KEY_RAW_DOT] = '>',
	[KEY_RAW_SLASH] = '?',
	[KEY_RAW_SPACE] = ' ',
	[KEY_RAW_PAD_1] = '1',
	[KEY_RAW_PAD_2] = '2',
	[KEY_RAW_PAD_3] = '3',
	[KEY_RAW_PAD_4] = '4',
	[KEY_RAW_PAD_5] = '5',
	[KEY_RAW_PAD_6] = '6',
	[KEY_RAW_PAD_7] = '7',
	[KEY_RAW_PAD_8] = '8',
	[KEY_RAW_PAD_9] = '9',
	[KEY_RAW_PAD_0] = '0',
	[KEY_RAW_PAD_SLASH] = '/',
	[KEY_RAW_PAD_MINUS] = '-',
	[KEY_RAW_PAD_STAR] = '*',
	[KEY_RAW_PAD_PLUS] = '+',
	[KEY_RAW_PAD_ENTER] = '\n',
	[KEY_RAW_PAD_DOT] = '.',
};

const char *defkeymap_fn[] = {
	[KEY_RAW_F1] = "\033OP",
	[KEY_RAW_F2] = "\033OQ",
	[KEY_RAW_F3] = "\033OR",
	[KEY_RAW_F4] = "\033OS",
	[KEY_RAW_F5] = "\033[15~",
	[KEY_RAW_F6] = "\033[17~",
	[KEY_RAW_F7] = "\033[18~",
	[KEY_RAW_F8] = "\033[19~",
	[KEY_RAW_F9] = "\033[20~",
	[KEY_RAW_F10] = "\033[21~",
	[KEY_RAW_F11] = "\033[23~",
	[KEY_RAW_F12] = "\033[24~",
	[KEY_RAW_INSERT] = "\033[2~",
	[KEY_RAW_HOME] = "\033[H",
	[KEY_RAW_PAGE_UP] = "\033[5~",
	[KEY_RAW_DELETE] = "\033[3~",
	[KEY_RAW_END] = "\033[F",
	[KEY_RAW_PAGE_DOWN] = "\033[6~",
	[KEY_RAW_ARROW_UP] = "\033[A",
	[KEY_RAW_ARROW_DOWN] = "\033[B",
	[KEY_RAW_ARROW_LEFT] = "\033[D",
	[KEY_RAW_ARROW_RIGHT] = "\033[C",
};

#endif
