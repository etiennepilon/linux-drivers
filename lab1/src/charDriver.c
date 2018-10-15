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
#define NB_DEVS     1
#define DEV_NAME    "char_driver_etsmtl"
#define BUFFER_SIZE 256
#define DEBUG 1
#define TEST 1
#define MAX_WRITER 1
#define MAX_READER 1

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
//TODO: static struct ioctl
// -- Variables --

static dev_t dev_num;
static int cd_minor = 0;
static struct class* cd_class;
static struct cdev cd_cdev;
static int number_opens = 0;
static char read_buffer[BUFFER_SIZE];
static char write_buffer[BUFFER_SIZE];
static cd_dev* _dev;

// -- Device Struct --
static struct file_operations cd_fops = {
    .owner = THIS_MODULE,
    .open = cd_open,
    .release = cd_release,
    .read = cd_read,
    .write = cd_write
};
// -- Private methods --
static cd_dev* cd_dev_create(void){
    char *read_buffer, *write_buffer;
    cd_dev* dev;
    dev = kmalloc(sizeof(cd_dev), GFP_KERNEL);
    if (dev == NULL) {
        D printk(KERN_WARNING"Error initializing device (%s:%s:%u)\n", __FILE__, __FUNCTION__, __LINE__);
        return NULL;
    }
    /* Init buffers */
    read_buffer = kmalloc(sizeof(char)*BUFFER_SIZE, GFP_KERNEL);
    write_buffer = kmalloc(sizeof(char)*BUFFER_SIZE, GFP_KERNEL);
    if(read_buffer == NULL || write_buffer == NULL){
        D printk(KERN_WARNING"Error initializing buffers (%s:%s:%u)\n", __FILE__, __FUNCTION__, __LINE__);
        return NULL;
    }
    dev->reader_cbuf = cbuf_init(read_buffer, BUFFER_SIZE);
    dev->writer_cbuf = cbuf_init(write_buffer, BUFFER_SIZE);
    if(dev->reader_cbuf == NULL || dev->writer_cbuf == NULL) 
    {
        D printk(KERN_WARNING"Error initializing circ buffers (%s:%s:%u)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    dev->num_reader = 0;
    dev->num_writer = 0;
    sema_init(&dev->sem, NB_DEVS);
    return dev;
}

static void cd_dev_destroy(cd_dev* dev){
    if (dev == NULL) return;
    cbuf_free(dev->reader_cbuf);
    cbuf_free(dev->writer_cbuf);
    kfree(dev);
}
// -- File operations --
static int __init cd_init(void){
    int result;
    // -- Create device major number --
    result = alloc_chrdev_region(&dev_num, cd_minor, NB_DEVS, DEV_NAME);
    if (result < 0 ) {
        D printk(KERN_WARNING"Error allocating handle number in alloc_chrdev_region\n");
        return result;
    } else {
        printk(KERN_WARNING"Char driver - Major:%u, Minor:%u\n", MAJOR(dev_num), MINOR(dev_num));
    }
    // -- Create device handle --
    cd_class = class_create(THIS_MODULE, "cdClass");
    device_create(cd_class, NULL/*no parent*/, dev_num, NULL, DEV_NAME);
    // -- Init and add cdev with fops
    cdev_init(&cd_cdev, &cd_fops);
    cd_cdev.owner = THIS_MODULE;
    result = cdev_add(&cd_cdev, dev_num, NB_DEVS);
    if (result < 0) {
        D printk(KERN_WARNING"Error adding char driver to the system");
        return result;
    }
    _dev = cd_dev_create();
    if(_dev == NULL) {
        D printk(KERN_WARNING"WTF");
    }
    /*
TODO: Serial interupt, addresses reservation for serial comm, Error if not successful
     */
    return result;
}

static void __exit cd_exit(void){
    cdev_del(&cd_cdev);
    unregister_chrdev_region(dev_num, NB_DEVS);
    device_destroy(cd_class, dev_num);
    class_destroy(cd_class);
    cd_dev_destroy(_dev);
    printk(KERN_WARNING"Char driver unregistered\n");
}

static int cd_open(struct inode *inode, struct file *flip){
    int retval = 0;
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
out:
   up(&_dev->sem);
   return 0;
}

static ssize_t cd_read(struct file *flip, char __user *ubuf, size_t count, loff_t *f_pos){
    int read_count = 0, i = 0;
    
    if(_dev == NULL) return -ENOENT; 
    if(flip->f_flags & O_NONBLOCK)
    {
        if(down_interruptible(&_dev->sem)) return -ERESTARTSYS;
        int buf_size = (int)cbuf_current_size(_dev->reader_cbuf);
        read_count = count < buf_size ? count : buf_size;
        for(i = 0; i < read_count; i ++)
        {
           cbuf_pop(_dev->reader_cbuf, read_buffer + i); 
        } 
        up(&_dev->sem);
    }
    else
    {
        // TODO: Block if read_count != count
        // current_size()!=count -> wait
        // This isn't the appropriate behaviour
        // How do I block users?
        if(down_interruptible(&_dev->sem)) return -ERESTARTSYS;
        int buf_size = (int)cbuf_current_size(_dev->reader_cbuf);
        read_count = count < buf_size ? count : buf_size;
        for(i = 0; i < read_count; i ++)
        {
           cbuf_pop(_dev->reader_cbuf, read_buffer + i); 
        } 
        up(&_dev->sem);
    }
    copy_to_user(ubuf, read_buffer, read_count);
    return (ssize_t)read_count;
}

static ssize_t cd_write(struct file *flip, const char __user *ubuf, size_t count, loff_t *f_pos){
    int write_count = 0, i = 0, buffer_size=0;
    if (_dev == NULL) return -ENOENT;
    // TODO: Must put Serial port in tx mode right away
    if(flip->f_flags & O_NONBLOCK)
    {
        buffer_size = count < BUFFER_SIZE? count: BUFFER_SIZE;
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
        buffer_size = count < BUFFER_SIZE? count: BUFFER_SIZE;
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

module_init(cd_init);
module_exit(cd_exit);


