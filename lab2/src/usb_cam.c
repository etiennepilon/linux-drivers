#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <uapi/asm-generic/errno-base.h>
#include <linux/fcntl.h>
#include <uapi/asm-generic/fcntl.h>
#include <linux/wait.h>
#include <linux/spinlock.h>
#include <linux/device.h>
#include <asm/atomic.h>
#include <asm/uaccess.h>
#include <uapi/asm-generic/ioctl.h>
#include <uapi/asm-generic/int-ll64.h>
#include <linux/usb.h>
#include <linux/completion.h>
#include "usb_cam.h"

// -- Module Basic Information --
MODULE_AUTHOR("Etienne Boudreault-Pilon");
MODULE_LICENSE("Dual BSD/GPL");

// -- Prototypes --
static int __init usb_cam_init(void);
static void __exit usb_cam_exit(void);
static int usb_cam_probe (struct usb_interface *intf, const struct usb_device_id *devid);
static void usb_cam_disconnect(struct usb_interface *intf);
static int usb_cam_open (struct inode *inode, struct file *filp);
static int usb_cam_release (struct inode *inode, struct file *filp) ;
static ssize_t usb_cam_read (struct file *filp, char __user *ubuf, size_t count, loff_t *f_ops);
static ssize_t usb_cam_write (struct file *filp, const char __user *ubuf, size_t count, loff_t *f_ops);
static long usb_cam_ioctl (struct file *filp, unsigned int cmd, unsigned long arg);
// -- Private methods --
// TODO


// -- Basic init & exit --
module_init(usb_cam_init);
module_exit(usb_cam_exit);

// -- Program stuctures --
// My usb Device
struct my_usb_cam_dev {
    struct usb_device *usb_dev;
    struct usb_interface *usb_cam_intf;
};

// fops
struct file_operations usb_cam_fops = {
	.owner          = THIS_MODULE,
	.open           = usb_cam_open,
	.release        = usb_cam_release,
	.read           = usb_cam_read,
	.write          = usb_cam_write,
	.unlocked_ioctl = usb_cam_ioctl,
}

// table
static struct usb_device_id usb_cam_table [] = {
	{USB_DEVICE(0x046d, 0x08cc)},
	{}
};
MODULE_DEVICE_TABLE(usb, usb_cam_table);

// driver
static struct usb_driver usb_cam_driver = {
	.name       = "usb_cam",
	.id_table   = usb_cam_table,
	.probe      = usb_cam_probe,
	.disconnect = usb_cam_disconnect,
};

// -- Program Variables --
// TODO

static int __init usb_cam_init(void)
{
	int retval = 0;
	retval = usb_register(&usb_cam_driver);
	if (retval)
	{
		printk(KERN_ERR"USB CAM INIT -- ERROR registering: %d\n", retval);
		return retval;
	}
	printk(KERN_WARNING"USB CAM INIT -- Registered\n");
	return retval;
}

static void __exit usb_cam_exit(void)
{
	printk(KERN_WARNING"USB CAM EXIT -- Deregistered\n");
	usb_deregister(&usb_cam_driver);
}
static int usb_cam_probe (struct usb_interface *intf, const struct usb_device_id *devid)
{
	int retval = 0;
	return retval;
}
static void usb_cam_disconnect(struct usb_interface *intf)
{
	//TODO
}
static int usb_cam_open (struct inode *inode, struct file *filp)
{
	int retval = 0;
	return retval;
}
static int usb_cam_release (struct inode *inode, struct file *filp)
{
	int retval = 0;
	return retval;
}
static ssize_t usb_cam_read (struct file *filp, char __user *ubuf, size_t count, loff_t *f_ops)
{
	int retval = 0;
	return retval;
}
static ssize_t usb_cam_write (struct file *filp, const char __user *ubuf, size_t count, loff_t *f_ops)
{
	int retval = 0;
	return retval;
}
static long usb_cam_ioctl (struct file *filp, unsigned int cmd, unsigned long arg)
{
	int retval = 0;
	return retval;
}

