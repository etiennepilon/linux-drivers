#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include "usb_cam.h"
#include "dht_data.h"

#define USB_HANDLE "/dev/etsele_cdev0"
#define DEFAULT_PATH "/home/ens/AK58050/workspaces/linux-drivers/lab2/src/pics/selfie.jpg"

unsigned char inBuffer[42666];
unsigned char finalBuffer[42666 * 2];

int take_picture(int usb_fd, FILE* pic_fd)
{
	int mySize = 0, retval = 0;
	mySize = read(usb_fd, inBuffer, 42666);
	if (mySize < 0)
	{
		printf("Error reading from usb cam\n");
		return -1;
	}
	
	if (retval < 0) return -1;
	memcpy(finalBuffer, inBuffer, HEADERFRAME1);
	memcpy(finalBuffer + HEADERFRAME1, dht_data, DHT_SIZE);
	memcpy(finalBuffer + HEADERFRAME1 + DHT_SIZE,
		inBuffer + HEADERFRAME1,
		(mySize - HEADERFRAME1));
	fwrite(finalBuffer, mySize + DHT_SIZE, 1, pic_fd);
	return 0;
}

int main(int argc, char* argv[])
{
	int retval = 0, direction = 0;
	int usb_fd = 0;
	FILE* pic_fd;
	
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
	else if (strcmp(argv[1], "selfie") == 0)
	{
		pic_fd = argc >= 3 ? fopen(argv[2], "wb") : fopen(DEFAULT_PATH, "wb");
		if (pic_fd == NULL)
		{
			printf("Error: Can't open picture file\n");
			return 0;
		}
		retval = ioctl(usb_fd, USB_CAM_IOCTL_STREAMON);
		if (retval < 0) goto out;
		retval = ioctl(usb_fd, USB_CAM_IOCTL_GRAB);
		if (retval < 0) goto out;
		//take_picture(usb_fd, pic_fd);
		retval = ioctl(usb_fd, USB_CAM_IOCTL_STREAMOFF);
out:
		fclose(pic_fd);
	}
	else if (strcmp(argv[1], "right") == 0)
	{
		direction = TILT_RIGHT;
		retval = ioctl(usb_fd, USB_CAM_IOCTL_PANTILT, &direction);
	}
	else if (strcmp(argv[1], "left") == 0)
	{
		direction = TILT_LEFT;
		retval = ioctl(usb_fd, USB_CAM_IOCTL_PANTILT, &direction);
	}
	else if (strcmp(argv[1], "up") == 0)
	{
		direction = TILT_UP;
		retval = ioctl(usb_fd, USB_CAM_IOCTL_PANTILT, &direction);
	}
	else if (strcmp(argv[1], "down") == 0)
	{
		direction = TILT_DOWN;
		retval = ioctl(usb_fd, USB_CAM_IOCTL_PANTILT, &direction);
	}
	else if (strcmp(argv[1], "reset") == 0)
	{
		retval = ioctl(usb_fd, USB_CAM_IOCTL_PANTILT_RESET);
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