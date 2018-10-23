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
#define CD_IOCTL_SETPARTIY _IOW(CD_IOCTL_MAGIC, 3, int) /*0-2*/
#define CD_IOCTL_GETBUFSIZE _IOR(CD_IOCTL_MAGIC, 4, int) /*5-8*/
#define CD_IOCTL_SETBUFSIZE _IOW(CD_IOCTL_MAGIC, 5, int) 

// -- SERIAL --
#define BAUD_RATE 9600

#define WLEN_5 0
#define WLEN_6 1
#define WLEN_7 2
#define WLEN_8 3

#define REG_RBR 0x00
#define REG_THR 0x00
#define REG_DLL 0x00
#define REG_DLM 0x01
#define REG_IER 0x01
#define REG_IIR 0x02
#define REG_FCR 0x02
#define REG_LCR 0x03
#define REG_MCR 0x04
#define REG_LSR 0x05
#define REG_MSR 0x06
#define REG_SCR 0x07

//config: DLM/DLR et lcr
// spin_lock for interupt
// check lsr in interupt handler
// request_region to get serial base address access
// always keep interupt enabled
// use inb/outb to write bytes at the address
typedef struct {
    int baud_rate;
    char parity_select;
    char parity_enabled;
    char stop_bit;
    char word_len_selection;
    int base_address;
    int irq_num;
} serial_config;

// -- DEVICE --
typedef struct {
    cbuf_handle_t reader_cbuf; /*From Port to User*/
    cbuf_handle_t writer_cbuf; /*From User to Port*/
    struct semaphore sem;
    wait_queue_head_t wait_queue;
    unsigned int num_reader;/*Used for release*/
    unsigned int num_writer;/*Used for release*/
    int buffer_size;
    struct cdev cdev;
    serial_config serial;
} cd_dev;

#endif
