/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 * $HEADER$
 * POSIX <termios.h> stub for the native Windows build -- enough for
 * pty-oriented code to compile; all operations fail with ENOTTY.
 */
#ifndef OPAL_WIN32_TERMIOS_H
#define OPAL_WIN32_TERMIOS_H

#include "opal_win32_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int   tcflag_t;
typedef unsigned char  cc_t;
typedef unsigned int   speed_t;

#define NCCS 32

struct termios {
    tcflag_t c_iflag;
    tcflag_t c_oflag;
    tcflag_t c_cflag;
    tcflag_t c_lflag;
    cc_t     c_cc[NCCS];
    speed_t  c_ispeed;
    speed_t  c_ospeed;
};

#define TCSANOW   0
#define TCSADRAIN 1
#define TCSAFLUSH 2

/* c_iflag bits */
#define IGNBRK  0x0001
#define BRKINT  0x0002
#define IGNPAR  0x0004
#define PARMRK  0x0008
#define INPCK   0x0010
#define ISTRIP  0x0020
#define INLCR   0x0040
#define IGNCR   0x0080
#define ICRNL   0x0100
#define IXON    0x0400
#define IXOFF   0x1000

/* c_lflag bits */
#define ICANON  0x0002
#define ECHO    0x0008
#define ECHOE   0x0010
#define ECHOK   0x0020
#define ECHONL  0x0040
#define ISIG    0x0001
#define OPOST   0x0001
#define ONLCR   0x0002
#define IEXTEN  0x8000
#define TOSTOP  0x0080
#define NOFLSH  0x0040
#define ECHOPRT 0x0200
#define ECHOCTL 0x0400
#define ECHOKE  0x0800

/* c_oflag bits */
#define OCRNL   0x0010
#define ONLRET  0x0020
#define NLDLY   0x0040
#define CRDLY   0x0600
#define TABDLY  0x1800
#define BSDLY   0x2000
#define VTDLY   0x4000
#define FFDLY   0x8000
#define VMIN    6
#define VTIME   5
#define VEOF    4
#define VEOL    5
#define VERASE  2
#define VINTR   0
#define VKILL   3
#define VQUIT   1
#define VSUSP   10
#define VSTART  8
#define VSTOP   9
#define B0      0
#define B9600   9600
#define B19200  19200
#define B38400  38400
/* c_cflag bits */
#define CS5     0x00
#define CS6     0x10
#define CS7     0x20
#define CS8     0x30
#define CSIZE   0x30
#define CSTOPB  0x40
#define CREAD   0x80
#define PARENB  0x100
#define PARODD  0x200
#define HUPCL   0x400
#define CLOCAL  0x800

OPAL_WIN32_DECLSPEC int tcgetattr(int fd, struct termios *termios_p);
OPAL_WIN32_DECLSPEC int tcsetattr(int fd, int optional_actions, const struct termios *termios_p);
OPAL_WIN32_DECLSPEC int tcsendbreak(int fd, int duration);
OPAL_WIN32_DECLSPEC int tcdrain(int fd);
OPAL_WIN32_DECLSPEC int tcflush(int fd, int queue_selector);
OPAL_WIN32_DECLSPEC int tcflow(int fd, int action);
OPAL_WIN32_DECLSPEC void cfmakeraw(struct termios *termios_p);
OPAL_WIN32_DECLSPEC speed_t cfgetispeed(const struct termios *termios_p);
OPAL_WIN32_DECLSPEC speed_t cfgetospeed(const struct termios *termios_p);
OPAL_WIN32_DECLSPEC int cfsetispeed(struct termios *termios_p, speed_t speed);
OPAL_WIN32_DECLSPEC int cfsetospeed(struct termios *termios_p, speed_t speed);
OPAL_WIN32_DECLSPEC int cfsetspeed(struct termios *termios_p, speed_t speed);

#define TCOOFF 0
#define TCOON  1
#define TCIOFF 2
#define TCION  3
#define TCIFLUSH   0
#define TCOFLUSH   1
#define TCIOFLUSH  2

#ifdef __cplusplus
}
#endif

#endif /* OPAL_WIN32_TERMIOS_H */
