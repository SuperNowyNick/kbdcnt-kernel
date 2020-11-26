#ifndef KBDCNT_H
#define KBDCNT_H

#include <linux/ioctl.h>

#define KBDCNT_NAME "kbdcnt"
#define KBDCNT_CLASS "kbdcnt"
#define KBDCNT_DEVICE "kbdcnt"

#define KBDCNT_MAJOR 241

// Ioctl routine to reset key press counter to zero and set new reset timestamp
#define IOCTL_RST _IO(KBDCNT_MAJOR, 0)
// Ioctl routine to obtain key press counter as unsigned long long
#define IOCTL_GET_CNT _IOR(KBDCNT_MAJOR, 1, unsigned long long)
// Ioctl routine to obtain last counter reset timestamp in ns from linux Epoch
#define IOCTL_GET_TIME _IOR(KBDCNT_MAJOR, 2, signed long long)

#endif
