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

#define clear() printf("\033[H\033[J")

typedef enum option {WRITE, READ, IOCTL, DONE} option_t;
char read_buffer[256] = {0}, write_buffer[256] = {0};

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
	int retval = 0;
	printf("Select mode:\nO_RDONLY(0)\nO_WRONLY(1)\nO_RDWR(2)\n");
	scanf("%d", &input);
	if (input <= 2)
	{
		if (input == 0) return O_RDONLY;
		else if (input == 1) return O_WRONLY;
		else return O_RDWR;
	}
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
option_t get_intention(void)
{
	unsigned int input = 0;
	printf("Select Intention:\nWrite(0)\nRead(1)\nIOCTL(2)\nExit(3)\n");
	scanf("%d", &input);
	if (input <= 2) return (option_t)input;
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

unsigned int get_buffer_size(void)
{
	unsigned int input = 0;
	printf("What's the buffer size?\n");
	scanf("%d", &input);
	if (input <= 1024) return input;
	else get_byte_count();
}

void user_input_to_buffer(int byte_count)
{
	int c;
	printf("Input here: ");
	while((c = getchar()) != '\n' && c != EOF);
	fgets(write_buffer, byte_count + 1, stdin);
}
unsigned int get_parity_mode(void)
{
	unsigned int input = 0;
	printf("No parity(0)\nParity Pair(1)\n Parity Impair(2)\n");
	scanf("%d", &input);
	if (input <= 2) return input;
	else get_parity_mode();
}

char isElement(int val, int* arr, int size)
{
	int i = 0;
	for(i = 0; i < size; i ++)
	{
		if (arr[i] == val) return 1;
	}
	return 0;
}

unsigned int get_baud_rate(void)
{
	unsigned int input = 0;
	unsigned int values[8] = {1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200};
	printf("Enter baud rate: 1200-115200\n");
	scanf("%d", &input);
	if (isElement(input, values, 8)) return input;
	else get_baud_rate();
}

unsigned int get_serial_size(void)
{
	unsigned int input = 0;
	unsigned int values[4] = {5, 6, 7, 8};
	printf("Enter baud rate: 5-8\n");
	scanf("%d", &input);
	if (isElement(input, values, 4)) return input;
	else get_serial_size();
}

unsigned int get_ioctl_choice(void)
{
	unsigned int input = 0;
	printf("Select Intention:\nSet Baud Rate(0)\nSet serial data size(1)\nset parity(2)\nGet buffer size(3)\nSet buffer size(4)\nDone(5)\n");
	scanf("%d", &input);
	if (input <= 5) return input;
	else get_ioctl_choice();
}

void manage_ioctl(int fd)
{
	int retval =0, ioctl_data_read = 0, ioctl_data_wr = 0;
	//clear();
	switch(get_ioctl_choice())
	{
	case 0:
		ioctl_data_wr = get_baud_rate();
		retval = ioctl(fd, CD_IOCTL_SETBAUDRATE, &ioctl_data_wr);
		break;
	case 1:
		ioctl_data_wr = get_serial_size();
		retval = ioctl(fd, CD_IOCTL_SETDATASIZE, &ioctl_data_wr);
		break;
	case 2:
		ioctl_data_wr = get_parity_mode();
		printf("%d", ioctl_data_wr);
		retval = ioctl(fd, CD_IOCTL_SETPARITY, &ioctl_data_wr);
		break;
	case 3:
		retval = ioctl(fd, CD_IOCTL_GETBUFSIZE, &ioctl_data_read);
		printf("Buffer size: %u\n", ioctl_data_read);
		break;
	case 4:
		ioctl_data_wr = get_buffer_size();
		retval = ioctl(fd, CD_IOCTL_SETBUFSIZE, &ioctl_data_wr);
		break;
	default:
		return;
		break;
	}
	manage_ioctl(fd);
}

void run_menu(int fd)
{
	int byte_count = 0, retval = 0, i = 0;
	switch(get_intention())
		{
		case WRITE:
			byte_count = get_byte_count();
			user_input_to_buffer(byte_count);
	//		for(i = 0; i < byte_count; i ++)
	//		{
	//			printf("%c", write_buffer[i]);
	//		}
			retval = write(fd, write_buffer, byte_count);
			printf("Wrote %u bytes\n", retval);
			break;
		case READ:
			byte_count = get_byte_count();
			retval = read(fd, read_buffer, byte_count);
			printf("Read %u bytes\n", retval);
			for (i = 0; i < retval ; i ++ )
			{
				printf("%c", read_buffer[i]);
			}
			printf("\n");
			break;
		case IOCTL:
			manage_ioctl(fd);
			break;
		case DONE:
			goto out;
			break;
		default:
			break;
		}
	run_menu(fd);
out:
	return;
}
int main(void)
{
	int fd = 0;
	unsigned int handle, mode, blocking;
	handle = get_handle();
	mode = get_mode();
	blocking = get_block_status();
	fd = open( handle == 0 ? HANDLE0 : HANDLE1, mode | (blocking == 0 ? 0 : O_NONBLOCK));
	if (fd < 0)
	{
		printf("Error opening the file handle - is it already opened?\n");
		return 0;
	}
	clear();
	printf("Opened %s\n", handle == 0 ? HANDLE0 : HANDLE1);
	run_menu(fd);
	close(fd);
}
