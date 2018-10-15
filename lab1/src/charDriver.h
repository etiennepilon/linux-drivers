#ifndef CHARDRIVER_H_
#define CHARDRIVER_H_

#include <linux/kernel.h>
#include "circularBuffer.h"

typedef struct {
    cbuf_handle_t reader_cbuf; /*From Port to User*/
    cbuf_handle_t writer_cbuf; /*From User to Port*/
    struct semaphore sem;
    unsigned int num_reader;/*Used for release*/
    unsigned int num_writer;/*Used for release*/
    struct cdev cdev;
} cd_dev;

#endif
