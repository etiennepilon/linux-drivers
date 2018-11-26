#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include "usb_cam.h"

#define USB_HANDLE "/dev/etsele_cdev0"

int main(int argc, char* argv[])
{
	int retval = 0;
	int usb_fd = 0;
	usb_fd = open(USB_HANDLE, O_RDONLY);
	if (argc < 2)
	{
		printf("Missing arguments %d\n", argc);
		return retval;
	}
	
	if (usb_fd < 0)
	{
		printf("Error opening with error code: %d\n", usb_fd);
		return usb_fd;
	}
	if (strcmp(argv[1], "streamon") == 0)
	{
		retval = ioctl(usb_fd, USB_CAM_IOCTL_STREAMON);
	}
	else if (strcmp(argv[1], "streamoff") == 0)
	{
		retval = ioctl(usb_fd, USB_CAM_IOCTL_STREAMOFF);
	}
	else
	{
		printf("Received invalid command %s\n", argv[1]);
	}
	if (retval < 0)
	{
		printf("Error executing command %s\n", argv[1]);
	}
	close(usb_fd);
	return retval;
}