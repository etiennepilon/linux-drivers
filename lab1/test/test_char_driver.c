#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
//#include <stdlib.h>
#include <stdio.h>

char read_buff[16];
char write_buff[16] = "Helloooo worldss";

int main(){
    int char_driver, retval=0, i = 0;
    char_driver = open("/dev/etsmtl_1", O_RDWR);
    retval = write(char_driver, write_buff, 8);
    printf("Bytes writen: %u\n", retval);
    retval = write(char_driver, write_buff + 8, 8);
    printf("Bytes writen: %u\n", retval);
    retval = read(char_driver, read_buff, 16);
    printf("Bytes read: %u\n", retval);
    for (i = 0; i < 16; i ++){
        printf("%c", read_buff[i]);
    }
    retval = close(char_driver);
    return 0;
}
