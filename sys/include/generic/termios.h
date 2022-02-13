#ifndef TERMIOS_H
#define TERMIOS_H

#define NCCS	11

typedef unsigned char cc_t;
typedef unsigned long speed_t;
typedef unsigned long tcflag_t;

struct termios {
	tcflag_t c_iflag;
	tcflag_t c_oflag;
	tcflag_t c_cflag;
	tcflag_t c_lflag;
	cc_t c_cc[NCCS];
};

#define VEOF	0
#define VEOL	1
#define VERASE	2
#define VINTR	3
#define VKILL	4
#define VMIN	5
#define VQUIT	6
#define VSTART	7
#define VSTOP	8
#define VSUSP	9
#define VTIME	10

/* c_iflag bits */
#define BRKINT	0000001
#define ICRNL	0000002
#define IGNBRK	0000004
#define IGNCR	0000010
#define INLCR	0000020
#define INPCK	0000040
#define ISTRIP	0000100
#define IXANY	0000200
#define IXOFF	0000400
#define IXON	0001000
#define PARMRK	0002000

/* c_oflag bits */
#define OPOST	0000001
#define ONLCR	0000002
#define OCRNL	0000004
#define ONOCR	0000010
#define ONLRET	0000020
#define OFDEL	0000040
#define OFILL	0000100
#define NLDLY	0000400
#define NL0	0000000
#define NL1	0000400
#define CRDLY	0003000
#define CR0	0000000
#define CR1	0001000
#define CR2	0002000
#define CR3	0003000
#define TABDLY	0014000
#define TAB0	0000000
#define TAB1	0004000
#define TAB2	0010000
#define TAB3	0014000
#define XTABS	0014000
#define BSDLY	0020000
#define BS0	0000000
#define BS1	0020000
#define VTDLY	0040000
#define VT0	0000000
#define VT1	0040000
#define FFDLY	0040000
#define FF0	0000000
#define FF1	0040000

/* Baud rates */
#define B0	0000000
#define B50	0000001
#define B75	0000002
#define B110	0000003
#define B134	0000004
#define B150	0000005
#define B200	0000006
#define B300	0000007
#define B600	0000010
#define B1200	0000011
#define B1800	0000012
#define B2400	0000013
#define B4800	0000014
#define B9600	0000015
#define B19200	0000016
#define B38400	0000017

/* c_cflag bits */
#define CSIZE	0000060
#define CS5	0000000
#define CS6	0000020
#define CS7	0000040
#define CS8	0000060
#define CSTOPB	0000100
#define CREAD	0000200
#define PARENB	0000400
#define PARODD	0001000
#define HUPCL	0002000
#define CLOCAL	0004000

/* c_lflag bits */
#define ECHO	0000001
#define ECHOE	0000002
#define ECHOK	0000004
#define ECHONL	0000010
#define ICANON	0000020
#define IEXTEN	0000040
#define ISIG	0000100
#define NOFLSH	0000200
#define TOSTOP	0000400

#define TCSANOW		0
#define TCSADRAIN	1
#define TCSAFLUSH	2

#endif
