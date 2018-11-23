#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#define USB_HANDLE "/dev/usb_cam0"

int main(int argc, char* argv[])
{
	int retval = 0;
	int usb_fd = 0;
	usb_fd = open(USB_HANDLE, O_RDONLY);
	if (usb_fd < 0)
	{
		printf("Error opening with error code: %d\n", usb_fd);
		return usb_fd;
	}
	
	return retval;
}