#ifndef USB_CAM_H_
#define USB_CAM_H_

#include <asm/ioctl.h>
// -- IOCTLs --
// Mostly from docs and notes

#define USB_CAM_IOC_MAGIC  'Z'

#define USB_CAM_IOCTL_GET 			   _IOWR(USB_CAM_IOC_MAGIC, 0x10, int)
#define USB_CAM_IOCTL_SET 			   _IOW(USB_CAM_IOC_MAGIC, 0x20, int)
#define USB_CAM_IOCTL_STREAMON         _IO(USB_CAM_IOC_MAGIC, 0x30)
#define USB_CAM_IOCTL_STREAMOFF        _IO(USB_CAM_IOC_MAGIC, 0x40)
#define USB_CAM_IOCTL_GRAB             _IO(USB_CAM_IOC_MAGIC, 0x50)
#define USB_CAM_IOCTL_PANTILT          _IOW(USB_CAM_IOC_MAGIC, 0x60, int)
#define USB_CAM_IOCTL_PANTILT_RESET    _IO(USB_CAM_IOC_MAGIC, 0x70)

#define USB_CAM_IOC_MAXNR 0x70

#define TILT_UP 0
#define TILT_DOWN 1
#define TILT_LEFT 2
#define TILT_RIGHT 3


#endif