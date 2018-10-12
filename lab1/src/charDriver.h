#ifndef CHARDRIVER_H_
#define CHARDRIVER_H_

#include <linux/kernel.h>
#include "circularBuffer.h"

typedef struct {
    cbuf_handle_t reader_buffer; /*From Port to User*/
    cbuf_handle_t writer_buffer; /*From User to Port*/
    struct semaphore sem;
    unsigned int num_readers;/*Used for release*/
    unsigned int num_writers;/*Used for release*/
} cd_dev;

#endif
