#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
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

void* blocking_read(void* vargp)
{
	int handle_1, retval = 0, i = 0;
	char read_buff[16];
	handle_1 = open("/dev/etsmtl_1", O_RDWR);
	retval = read(handle_1, read_buff, 11);
	for (i = 0; i < retval; i ++)
	{
		printf("%c", read_buff[i]);
	}
	printf("\n");
    close(handle_1);
}
// NOTE
// This test can only be executed if a serial cable is plugged between serial ports
int main(){
    int handle_0, handle_1, retval=0, i = 0, ioctl_data_read = 0, ioctl_data_wr = 0;
    char read_buff[16];
    char write_buff[16] = "Hello world";
    pthread_t tid;
    // -----------------------------
    // -- Test for Open & Release --
    handle_0 = open("/dev/etsmtl_0", O_RDWR);
    handle_1 = open("/dev/etsmtl_0", O_RDWR);
    assert(handle_1 < 0);
    close(handle_0);
    handle_0 = open("/dev/etsmtl_0", O_RDONLY);
    handle_1 = open("/dev/etsmtl_0", O_RDWR);
    assert(handle_1 < 0);
    close(handle_0);
    handle_0 = open("/dev/etsmtl_0", O_WRONLY);
    handle_1 = open("/dev/etsmtl_0", O_RDWR);
    assert(handle_1 < 0);
    close(handle_0);
    // -----------------------------
    // ------- Test for non blocking Read -------
    handle_0 = open("/dev/etsmtl_0", O_RDWR | O_NONBLOCK);
    handle_1 = open("/dev/etsmtl_1", O_RDWR | O_NONBLOCK);

    retval = write(handle_0, write_buff, 8);
    assert(retval > 0);
    retval = write(handle_0, write_buff + retval, 11-retval);
    assert(retval > 0);
    usleep(500000000);
    retval = read(handle_1, read_buff, 16);
    printf("Following string should be Hello world: ");
    for(i = 0; i < retval; i ++)
    {
    	assert(write_buff[i] == read_buff[i]);
    	printf("%c", read_buff[i]);
    }
    printf("\n");

    close(handle_0);
    close(handle_1);
    usleep(50000);
    // -----------------------------
    // ------- Test for blocking Read -------
    handle_0 = open("/dev/etsmtl_0", O_RDWR | O_NONBLOCK);
    pthread_create(&tid, 0, blocking_read, 0);
    printf("This should come before the hello world message\n");

    retval = write(handle_0, write_buff, 11);

    pthread_join(tid, NULL);
    close(handle_0);
    usleep(50000);
    // --------------------------------------
    // ----- Test for IOCTL ----
    handle_0 = open("/dev/etsmtl_0", O_RDWR);
    retval = ioctl(handle_0, CD_IOCTL_GETBUFSIZE, &ioctl_data_read);
    printf("Buffer size: %u\n", ioctl_data_read);
    ioctl_data_wr = 512;
    retval = ioctl(handle_0, CD_IOCTL_SETBUFSIZE, &ioctl_data_wr);
    if (retval < 0) printf("Don't have admin right on these computers\n");
    ioctl_data_wr = 1;
    retval = ioctl(handle_0, CD_IOCTL_SETPARITY, &ioctl_data_wr);
    usleep(5000000);
    ioctl_data_wr = 0;
    retval = ioctl(handle_0, CD_IOCTL_SETPARITY, &ioctl_data_wr);
//    retval = ioctl(handle_0, CD_IOCTL_GETBUFSIZE, baud);
//    printf("Baud: %u", baud[0]);

    close(handle_0);
    return 0;
}
