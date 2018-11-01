#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>

char read_buff[16];
char write_buff[16] = "Helloooo worldss";

int main(){
    int handle_0, handle_1, retval=0, i = 0;
    handle_0 = open("/dev/etsmtl_0", O_RDWR | O_NONBLOCK);
    handle_1 = open("/dev/etsmtl_1", O_RDWR | O_NONBLOCK);

    retval = write(handle_0, write_buff, 6);
    usleep(50000);

    retval = read(handle_1, read_buff, 5);
    printf("Bytes read: %u handle_0\n", retval);
    for (i = 0; i < retval; i ++){
        printf("%c", read_buff[i]);
    }
    printf("\n");
    /*
    printf("Wrote %u bytes to handle 0\n", retval);
    retval = write(handle_1, write_buff + 1, 1);
    printf("Wrote %u bytes to handle 1\n", retval);
    usleep(50000);

    retval = read(handle_0, read_buff, 1);
    printf("Bytes read: %u handle_0\n", retval);
    for (i = 0; i < retval; i ++){
        printf("%u", read_buff[i]);
    }

    usleep(50000);

    retval = read(handle_1, read_buff, 1);
    printf("Bytes read: %u handle_1\n", retval);
    for (i = 0; i < retval; i ++){
        printf("%u", read_buff[i]);
    }
	*/
    close(handle_0);
    close(handle_1);
    return 0;
}
