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
#include <linux/kernel.h>
#include <uapi/asm-generic/errno-base.h>
#include <linux/fcntl.h>
#include <linux/wait.h>
#include <linux/spinlock.h>
#include <linux/device.h>
#include <asm/atomic.h>
#include <asm/uaccess.h>
// -- Configs --
#define NB_DEVS     1
#define DEV_NAME    "char_driver_etsmtl"

// -- Infos --
MODULE_AUTHOR("Etienne B.-Pilon");
MODULE_LICENSE("Dual BSD/GPL");

// -- Prototypes --
static int __init cd_init(void);
static void __exit cd_exit(void);
static int cd_open(struct inode *inode, struct file *flip);
static int cd_release(struct inode *inode, struct file *flip);
static ssize_t cd_read(struct file *flip, char __user *ubuf, size_t count, loff_t *f_ops);
static ssize_t cd_write(struct file *flip, const char __user *ubuf, size_t count, loff_t *f_ops);
//TODO: static struct ioctl
// -- Variables --

dev_t dev_num;
int cd_minor = 0;
struct class* cd_class;
struct cdev cd_cdev;

// -- Device Struct --
static struct file_operations cd_fops = {
    .owner = THIS_MODULE,
    .open = NULL,
    .release = NULL,
    .read = NULL,
    .write = NULL
//TODO:  .unlocked_ioctl = 
};

static int __init cd_init(void){
    int result;
    // -- Create device major number --
    result = alloc_chrdev_region(&dev_num, cd_minor, NB_DEVS, DEV_NAME);
    if (result < 0 ) {
        printk(KERN_WARNING"Error allocating handle number in alloc_chrdev_region\n");
        return result;
    } else {
        printk(KERN_WARNING"Char driver - Major:%u, Minor:%u\n", MAJOR(dev_num), MINOR(dev_num));
    }
    // -- Create device handle --
    cd_class = class_create(THIS_MODULE, "cdClass");
    // -- Warning: Source code differs from official doc for device_create
    // See: /usr/src/linux-headers-4.15.18+/include/linux/device.h:1201 
    //   for correct implementation
    device_create(cd_class, NULL/*no parent*/, dev_num, NULL, DEV_NAME);
    // -- Init and add cdev with fops
    cdev_init(&cd_cdev, &cd_fops);
    cd_cdev.owner = THIS_MODULE;
    result = cdev_add(&cd_cdev, dev_num, NB_DEVS);
    if (result < 0) {
        printk(KERN_WARNING"Error adding char driver to the system");
        return result;
    }
    return 0;
}

static void __exit cd_exit(void){
    cdev_del(&cd_cdev);
    unregister_chrdev_region(dev_num, NB_DEVS);
    device_destroy(cd_class, dev_num);
    class_destroy(cd_class);
    printk(KERN_WARNING"Char driver unregistered\n");
}

module_init(cd_init);
module_exit(cd_exit);


