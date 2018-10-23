/*
 * =====================================================================================
 *
 *       Filename:  charDriver.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  04/10/2018 15:00:26
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Etienne B.-Pilon BOUE07079001
 *   Organization:  
 *
 * =====================================================================================
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/errno.h>
//#include <linux/kernel.h>
#include <uapi/asm-generic/errno-base.h>
#include <linux/fcntl.h>
#include <linux/wait.h>
#include <linux/spinlock.h>
#include <linux/device.h>
#include <asm/atomic.h>
#include <linux/uaccess.h>
#include "charDriver.h"
#include "circularBuffer.h"
// -- Configs --
#define NB_DEVS     2
#define DEV_NAME    "etsmtl_serial"
#define DEF_BUFFER_SIZE 256
#define DEBUG 1
#define TEST 1
#define MAX_WRITER 1
#define MAX_READER 1

#define PORT0_IRQ 20
#define PORT0_BASE_ADDR 0xc020
#define PORT1_IRQ 21
#define PORT1_BASE_ADDR 0xc030

#ifdef DEBUG
    #define D if(1)
#else
    #define D if(0)
#endif

#ifdef TEST
    #define T if(1)
#else
    #define T if(0)
#endif
// -- Infos --
MODULE_AUTHOR("Etienne B.-Pilon");
MODULE_LICENSE("Dual BSD/GPL");

// -- Prototypes --
static int __init cd_init(void);
static void __exit cd_exit(void);
static int cd_open(struct inode *inode, struct file *flip);
static int cd_release(struct inode *inode, struct file *flip);
static ssize_t cd_read(struct file *flip, char __user *ubuf, size_t count, loff_t *f_pos);
static ssize_t cd_write(struct file *flip, const char __user *ubuf, size_t count, loff_t *f_pos);
static long cd_ioctl(struct file* flip, unsigned int cmd, unsigned long arg);
// -- Serial prototypes --
// static serial_irq(whatever);
//TODO: Manage two handles (Create two class)
//  each class should have their dev
// -- Variables --

static dev_t dev_num;
static int cd_minor = 0;
static struct class* cd_class0;
static struct class* cd_class1;
static struct cdev cd_cdev;
static char read_buffer[DEF_BUFFER_SIZE];
static char write_buffer[DEF_BUFFER_SIZE];
//static cd_dev* _dev;
static cd_dev* _dev_0;
static cd_dev* _dev_1;

// -- Device Struct --
static struct file_operations cd_fops = {
    .owner = THIS_MODULE,
    .open = cd_open,
    .release = cd_release,
    .read = cd_read,
    .write = cd_write,
    .unlocked_ioctl = cd_ioctl
};

static int get_irq_for_minor(int minor)
{
    if (minor == 0) return PORT0_IRQ;
    if (minor == 1) return PORT1_IRQ;
    D printk(KERN_WARNING"Missing IRQ configs for minor: %u\n", minor);
    return -1;
}

static int get_base_address_for_minor(int minor)
{
    if (minor == 0) return PORT0_BASE_ADDR;
    if (minor == 1) return PORT1_BASE_ADDR;
    D printk(KERN_WARNING"Missing Base address configs for minor: %u\n", minor);
    return -1;
}

static void init_serial_port(cd_dev* dev, int irq_num, int base_address)
{
    dev->serial.baud_rate = BAUD_RATE;
    dev->serial.parity_select = 0;
    dev->serial.parity_enabled= 0;
    dev->serial.stop_bit = 0;
    dev->serial.word_len_selection=WLEN_8;
    dev->serial.base_address = base_address;
    dev->serial.irq_num = irq_num;
    return;
}

// -- Private methods --
static cd_dev* cd_dev_create(int irq_num, int base_address){
    char *read_buffer, *write_buffer;
    cd_dev* dev;
    dev = kmalloc(sizeof(cd_dev), GFP_KERNEL);
    if (dev == NULL) {
        D printk(KERN_WARNING"Error initializing device (%s:%s:%u)\n", __FILE__, __FUNCTION__, __LINE__);
        return NULL;
    }
    /* Init buffers */
    read_buffer = kmalloc(sizeof(char)*DEF_BUFFER_SIZE, GFP_KERNEL);
    write_buffer = kmalloc(sizeof(char)*DEF_BUFFER_SIZE, GFP_KERNEL);
    if(read_buffer == NULL || write_buffer == NULL){
        D printk(KERN_WARNING"Error initializing buffers (%s:%s:%u)\n", __FILE__, __FUNCTION__, __LINE__);
        kfree(dev);
        return NULL;
    }
    dev->reader_cbuf = cbuf_init(read_buffer, DEF_BUFFER_SIZE);
    dev->writer_cbuf = cbuf_init(write_buffer, DEF_BUFFER_SIZE);
    if(dev->reader_cbuf == NULL || dev->writer_cbuf == NULL) 
    {
        D printk(KERN_WARNING"Error initializing circ buffers (%s:%s:%u)\n", __FILE__, __FUNCTION__, __LINE__);
        kfree(dev);
        return NULL;
    }
    init_waitqueue_head(&dev->wait_queue);
    dev->buffer_size = DEF_BUFFER_SIZE;
    dev->num_reader = 0;
    dev->num_writer = 0;
    init_serial_port(dev, irq_num, base_address);
    sema_init(&dev->sem, 1);
    return dev;
}

static void cd_dev_destroy(cd_dev* dev){
    if (dev == NULL) return;
    cbuf_free(dev->reader_cbuf);
    cbuf_free(dev->writer_cbuf);
    kfree(dev);
}

// TODO: Wait event queue with timeout to test in/out of serial port
// -- File operations --
static int __init cd_init(void){
    int result;
    // -- Create device major number --
    result = alloc_chrdev_region(&dev_num, cd_minor, NB_DEVS, DEV_NAME);
    if (result < 0 ) {
        D printk(KERN_WARNING"Error allocating handle number in alloc_chrdev_region\n");
        return result;
    } 
    // -- Create device handle --
    cd_class0 = class_create(THIS_MODULE, "serialClass0");
    device_create(cd_class0, NULL/*no parent*/, dev_num, NULL, "etsmtl_0");
    cd_class1 = class_create(THIS_MODULE, "serialClass1");
    device_create(cd_class1, NULL/*no parent*/, dev_num + 1, NULL, "etsmtl_1");
    
    // -- Init and add cdev with fops
    cdev_init(&cd_cdev, &cd_fops);
    cd_cdev.owner = THIS_MODULE;
    result = cdev_add(&cd_cdev, dev_num, NB_DEVS);
    if (result < 0) {
        D printk(KERN_WARNING"Error adding char driver to the system");
        return result;
    }
//    _dev = cd_dev_create(PORT0_IRQ, PORT0_BASE_ADDR);
    _dev_0 = cd_dev_create(PORT0_IRQ, PORT0_BASE_ADDR);
    _dev_1 = cd_dev_create(PORT1_IRQ, PORT1_BASE_ADDR);
    if(_dev_0 == NULL ||_dev_1 == NULL ) {
        D printk(KERN_WARNING"Error creating device.");
        return -ENOTTY;
    }
    return result;
}

static void __exit cd_exit(void){
    int i = 0;
    cdev_del(&cd_cdev);
    unregister_chrdev_region(dev_num, NB_DEVS);
    device_destroy(cd_class0, dev_num);
    class_destroy(cd_class0);
    device_destroy(cd_class1, dev_num+1);
    class_destroy(cd_class1);
  //  cd_dev_destroy(_dev);
    cd_dev_destroy(_dev_0);
    cd_dev_destroy(_dev_1);
    printk(KERN_WARNING"Char driver unregistered\n");
}

static int cd_open(struct inode *inode, struct file *flip){
    int retval = 0;
    cd_dev* _dev = MINOR(inode->i_rdev) == 0 ? _dev_0: _dev_1;
   D printk(KERN_WARNING"charDriver opened\n");
   
   if(down_interruptible(&_dev->sem)) return -ERESTARTSYS;
   switch((flip->f_flags & O_ACCMODE)){
       case O_RDONLY:
           if(_dev->num_writer >= MAX_WRITER || _dev->num_reader >= MAX_READER) 
           {
               retval = -ENOTTY;
               goto out;
           }
           _dev->num_reader++;
           // TODO: Put Serial port to open/receive so it can start receiving
           // data
           break;
       case O_WRONLY:
           if(_dev->num_writer >= MAX_WRITER || _dev->num_reader >= MAX_READER) 
           {
               retval = -ENOTTY;
               goto out;
           }
           _dev->num_writer++;
           break;
       case O_RDWR:
           if(_dev->num_writer >= MAX_WRITER || _dev->num_reader >= MAX_READER) 
           {
               retval = -ENOTTY;
               goto out;
           }
           _dev->num_reader++;
           _dev->num_writer++;
           // TODO: Put Serial port to open/receive so it can start receiving
           // data
           break;
       default:
           break;
   }
out:
   up(&_dev->sem);
   return retval;
}

static int cd_release(struct inode *inode, struct file *flip){
    cd_dev* _dev = MINOR(inode->i_rdev) == 0 ? _dev_0: _dev_1;
   if(down_interruptible(&_dev->sem)) return -ERESTARTSYS;
   switch((flip->f_flags & O_ACCMODE)){
       case O_RDONLY:
           _dev->num_reader--;
           // TODO: Put Serial port to close so it can start receiving
           // data
           break;
       case O_WRONLY:
           _dev->num_writer--;
           break;
       case O_RDWR:
           _dev->num_reader--;
           _dev->num_writer--;
           // TODO: Put Serial port to close so it can start receiving
           // data
           break;
       default:
           break;
   }
//out:
   up(&_dev->sem);
   return 0;
}

static ssize_t cd_read(struct file *flip, char __user *ubuf, size_t count, loff_t *f_pos){
    int read_count = 0, i = 0, buf_size = 0, blocking = 0;
    cd_dev* _dev = MINOR(flip->f_inode->i_rdev) == 0 ? _dev_0: _dev_1;
    D printk(KERN_WARNING"Minor num: %u", MINOR(flip->f_inode->i_rdev));
    if(_dev == NULL) return -ENOENT; 
    if(down_interruptible(&_dev->sem)) return -ERESTARTSYS;
    blocking = !(flip->f_flags & O_NONBLOCK);
    if (blocking)
    {
        while(cbuf_current_size(_dev->reader_cbuf) < count)
        {
            up(&_dev->sem);
            if(wait_event_interruptible(
                _dev->wait_queue,
                (cbuf_current_size(_dev->reader_cbuf) >= count)
                )) return -ERESTARTSYS;
            if(down_interruptible(&_dev->sem)) return -ERESTARTSYS;
        }
    }
    buf_size = (int)cbuf_current_size(_dev->reader_cbuf);
    read_count = count < buf_size ? count : buf_size;
    for(i = 0; i < read_count; i ++)
    {
        // TODO: Catch error
       cbuf_pop(_dev->reader_cbuf, read_buffer + i); 
    } 
    copy_to_user(ubuf, read_buffer, read_count);
    up(&_dev->sem);
    return (ssize_t)read_count;
}

static ssize_t cd_write(struct file *flip, const char __user *ubuf, size_t count, loff_t *f_pos){
    int write_count = 0, i = 0, buffer_size=0;
    cd_dev* _dev = MINOR(flip->f_inode->i_rdev) == 0 ? _dev_0: _dev_1;
    if (_dev == NULL) return -ENOENT;
    // TODO: Must put Serial port in tx mode right away
    if(flip->f_flags & O_NONBLOCK)
    {
        buffer_size = count < _dev->buffer_size? count: _dev->buffer_size;
        copy_from_user(write_buffer, ubuf, buffer_size);
        if(down_interruptible(&_dev->sem)) return -ERESTARTSYS;
        for (i = 0; i < buffer_size; i ++) {
            if(cbuf_is_full(_dev->writer_cbuf)) break;
            cbuf_put(_dev->writer_cbuf, write_buffer[i]);
            write_count += 1;
            // -- TEST --
            T cbuf_put(_dev->reader_cbuf, write_buffer[i]);
        }
        up(&_dev->sem);
    }
    else
    {
        // TODO: Add blocking call when count != cbuf_current_size
        buffer_size = count < _dev->buffer_size? count: _dev->buffer_size;
        copy_from_user(write_buffer, ubuf, buffer_size);
        if(down_interruptible(&_dev->sem)) return -ERESTARTSYS;
        for (i = 0; i < buffer_size; i ++) {
            if(cbuf_is_full(_dev->writer_cbuf)) break;
            cbuf_put(_dev->writer_cbuf, write_buffer[i]);
            write_count += 1;
            // -- TEST --
            T cbuf_put(_dev->reader_cbuf, write_buffer[i]);
        }
        up(&_dev->sem);
    }
    return write_count;
}

static long cd_ioctl(struct file* flip, unsigned int cmd, unsigned long arg){
    int err = 0, retval = 0, new_size = 0;
    cd_dev* _dev = MINOR(flip->f_inode->i_rdev) == 0 ? _dev_0: _dev_1;
    if(_IOC_TYPE(cmd) != CD_IOCTL_MAGIC || _IOC_NR(cmd) > CD_IOCTL_MAX) return -ENOTTY;
    if(_IOC_DIR(cmd) & _IOC_READ) 
        err = !access_ok(VERIFY_WRITE, (void __user*)arg, _IOC_SIZE(cmd));
    if(_IOC_DIR(cmd) & _IOC_WRITE) 
        err = !access_ok(VERIFY_READ, (void __user*)arg, _IOC_SIZE(cmd));
    if (err) return -EFAULT;
    switch(cmd){
        case CD_IOCTL_SETBAUDRATE:
            // TODO
            break;
        case CD_IOCTL_SETDATASIZE:
            break;
        case CD_IOCTL_SETPARTIY:
            // TODO
            break;
        case CD_IOCTL_GETBUFSIZE:
            retval = __put_user(_dev->buffer_size, (int __user*) arg); 
            break;
        case CD_IOCTL_SETBUFSIZE:
            if (!capable(CAP_SYS_ADMIN)) return -EPERM;
            retval = __get_user(new_size, (int __user*) arg);
            if (retval < 0) return retval;
            if (new_size < 128 || new_size > 4096) return -ENOTTY;
            if(down_interruptible(&_dev->sem)) return -ERESTARTSYS;
            retval = cbuf_resize(_dev->reader_cbuf, new_size);
            if (retval < 0) goto outsem;
            retval = cbuf_resize(_dev->writer_cbuf, new_size);
            if (retval < 0) goto outsem;
            _dev->buffer_size = new_size;
outsem:
            up(&_dev->sem);
            break;
        default: return -ENOTTY;
    }
    return retval;
}

module_init(cd_init);
module_exit(cd_exit);
