#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>

#define CD_IOCTL_MAGIC 'k'
#define CD_IOCTL_MAX 5

#define CD_IOCTL_SETBAUDRATE _IOW(CD_IOCTL_MAGIC, 1, int) /*50-115200*/
#define CD_IOCTL_SETDATASIZE _IOW(CD_IOCTL_MAGIC, 2, int) /*5-8*/
#define CD_IOCTL_SETPARITY _IOW(CD_IOCTL_MAGIC, 3, int) /*0-2*/
#define CD_IOCTL_GETBUFSIZE _IOR(CD_IOCTL_MAGIC, 4, int) /*5-8*/
#define CD_IOCTL_SETBUFSIZE _IOW(CD_IOCTL_MAGIC, 5, int)

#define HANDLE0 "/dev/etsmtl_0"
#define HANDLE1 "/dev/etsmtl_1"


unsigned int get_handle(void)
{
	unsigned int input = 0;
	printf("Select handle:\n/dev/etsmtl_0(0)\n/dev/etsmtl_1(1)\n");
	scanf("%d", &input);
	if (input <= 1) return input;
	else get_handle();
}

unsigned int get_mode(void)
{
	unsigned int input = 0;
	printf("Select mode:\nO_RDONLY(0)\nO_WRONLY(1)\nO_RDWR(2)\n");
	scanf("%d", &input);
	if (input <= 2) return input;
	else get_mode();
}

unsigned int get_block_status(void)
{
	unsigned int input = 0;
	printf("Select block:\nBlocking(0)\nNon-Blocking(1)\n");
	scanf("%d", &input);
	if (input <= 1) return input;
	else get_block_status();
}
unsigned int get_intention(void)
{
	unsigned int input = 0;
	printf("Select Intention:\nWrite(0)\nRead(1)\nIOCTL(2)\n");
	scanf("%d", &input);
	if (input <= 2) return input;
	else get_intention();
}

unsigned int get_byte_count(void)
{
	unsigned int input = 0;
	printf("How many characters?\n");
	scanf("%d", &input);
	if (input <= 256) return input;
	else get_byte_count();
}

void get_user_input(char* b)
{
	printf("Input here:\n");
	scanf("%[^\n]%*c", b);
}

int main(void)
{
	int fd = 0, byte_count = 0, retval = 0, i = 0, intention = 0;
	char read_buffer[256], write_buffer[256];
	fd = open(get_handle() == 0 ? HANDLE0 : HANDLE1,
			get_mode() | get_block_status() == 0 ? 0 : O_NONBLOCK);
	switch(get_intention())
	{
	case 0:
		byte_count = get_byte_count();
		get_user_input(write_buffer);
		retval = write(fd, write_buffer, byte_count);
		printf("Wrote %u bytes\n", retval);
		break;
	case 1:
		byte_count = get_byte_count();
		retval = read(fd, read_buffer, byte_count);
		printf("Read %u bytes\n", retval);
		for (i = 0; i < retval ; i ++ )
		{
			printf("%c", read_buffer[i]);
		}
		printf("\n");
		break;
	default:
		break;
	}

	close(fd);
}
