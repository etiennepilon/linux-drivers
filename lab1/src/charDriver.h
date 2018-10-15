#ifndef CHARDRIVER_H_
#define CHARDRIVER_H_

#include <linux/kernel.h>
#include <asm/ioctl.h>
#include "circularBuffer.h"

#define CD_IOCTL_MAGIC 'k'
#define CD_IOCTL_MAX 5

#define CD_IOCTL_SETBAUDRATE _IOW(CD_IOCTL_MAGIC, 1, int) /*50-115200*/
#define CD_IOCTL_SETDATASIZE _IOW(CD_IOCTL_MAGIC, 2, int) /*5-8*/
#define CD_IOCTL_SETPARTIY _IOW(CD_IOCTL_MAGIC, 3, int) /*0-2*/
#define CD_IOCTL_GETBUFSIZE _IOR(CD_IOCTL_MAGIC, 4, int) /*5-8*/
#define CD_IOCTL_SETBUFSIZE _IOW(CD_IOCTL_MAGIC, 5, int) 

typedef struct {
    cbuf_handle_t reader_cbuf; /*From Port to User*/
    cbuf_handle_t writer_cbuf; /*From User to Port*/
    struct semaphore sem;
    unsigned int num_reader;/*Used for release*/
    unsigned int num_writer;/*Used for release*/
    int buffer_size;
    struct cdev cdev;
} cd_dev;

#endif
