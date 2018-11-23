#ifndef CHARDRIVER_H_
#define CHARDRIVER_H_

#include <linux/kernel.h>
#include <asm/ioctl.h>
#include "circularBuffer.h"

// -- IOCTL --
#define CD_IOCTL_MAGIC 'k'
#define CD_IOCTL_MAX 5

#define CD_IOCTL_SETBAUDRATE _IOW(CD_IOCTL_MAGIC, 1, int) /*50-115200*/
#define CD_IOCTL_SETDATASIZE _IOW(CD_IOCTL_MAGIC, 2, int) /*5-8*/
#define CD_IOCTL_SETPARITY _IOW(CD_IOCTL_MAGIC, 3, int) /*0-2*/
#define CD_IOCTL_GETBUFSIZE _IOR(CD_IOCTL_MAGIC, 4, int) /*5-8*/
#define CD_IOCTL_SETBUFSIZE _IOW(CD_IOCTL_MAGIC, 5, int) 

// -- SERIAL --
#define BAUD_RATE 9600
#define SERIAL_CLK 1843200

#define WLEN_5 5
#define WLEN_6 6
#define WLEN_7 7
#define WLEN_8 8

#define RBR 0x00
#define THR 0x00
#define DLL 0x00
#define DLM 0x01
#define IER 0x01
#define IER_ETBEI 0x02
#define IER_ERBFI 0x01

#define IIR 0x02
#define FCR 0x02
#define FCR_RCVRTRM 0x80
#define FCR_RCVRTRL 0x40
#define FCR_RCVRRE 0x02
#define FCR_FIFOEN 0x01

#define LCR 0x03
#define LCR_DLAB 0x80
#define LCR_EPS 0x10
#define LCR_PEN 0x08
#define LCR_STB 0x04
#define LCR_WLS1 0x02
#define LCR_WLS0 0x01

#define MCR 0x04
#define LSR 0x05
#define LSR_TEMT 0x40
#define LSR_THRE 0x20
#define LSR_FE 0x08
#define LSR_PE 0x04
#define LSR_OE 0x02
#define LSR_DR 0x01

#define MSR 0x06
#define SCR 0x07

typedef struct {
    unsigned int baud_rate;
    char parity_select;
    char parity_enabled;
    char stop_bit;
    char word_len_selection;
    unsigned short int base_address;
    unsigned int irq_num;
} serial_config;

// -- DEVICE --
typedef struct {
    cbuf_handle_t reader_cbuf; /*From Port to User*/
    cbuf_handle_t writer_cbuf; /*From User to Port*/
    struct semaphore sem;
    spinlock_t lock;
    wait_queue_head_t wait_queue;
    unsigned int num_reader;/*Used for release*/
    unsigned int num_writer;/*Used for release*/
    unsigned int buffer_size;
    struct cdev cdev;
    serial_config serial;
} cd_dev;

#endif
