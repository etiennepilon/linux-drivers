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
#include "usbvideo.h"
#include "dht_data.h"

// -- Module Basic Information --
MODULE_AUTHOR("Etienne Boudreault-Pilon");
MODULE_LICENSE("Dual BSD/GPL");

// -- Constants --
#define MINOR_NUM 0

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
struct usb_cam_dev {
    struct usb_device *usb_dev;
    struct usb_interface *usb_cam_intf;
    struct urb *usb_can_urbs[5];
    atomic_t urb_counter;
};

// fops
struct file_operations usb_cam_fops = {
	.owner          = THIS_MODULE,
	.open           = usb_cam_open,
	.release        = usb_cam_release,
	.read           = usb_cam_read,
	.write          = usb_cam_write,
	.unlocked_ioctl = usb_cam_ioctl,
};

// table
static struct usb_device_id usb_cam_table[] = {
	{ USB_DEVICE(0x046d, 0x08cc) },
	{ USB_DEVICE(0x046d, 0x0994) },
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

// class
static struct usb_class_driver usb_cam_class = {
	.name       = "usb/usb_cam%d",
	.fops       = &usb_cam_fops,
	.minor_base = MINOR_NUM,
};

// -- Program Variables --
// See lab document
unsigned int myStatus = 0;
unsigned int myLength = 42666;
unsigned int myLengthUsed = 0;


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
	int retval = -1, streaming_intf_detected = 0;
	struct usb_device *dev = interface_to_usbdev(intf);
	struct usb_cam_dev *my_usb_dev = NULL;
	
	printk(KERN_WARNING"USB CAM PROBE\n");
	// -- USB Device class/subclass detection --
	if (intf->altsetting->desc.bInterfaceClass == CC_VIDEO)
	{
	    if (intf->altsetting->desc.bInterfaceSubClass == SC_VIDEOCONTROL)
	    {
	    	printk(KERN_WARNING"- video control interface detected\n");
	    	return 0;	
	    }
	    if (intf->altsetting->desc.bInterfaceSubClass == SC_VIDEOSTREAMING)
		{
	    	printk(KERN_WARNING"- video streaming interface detected\n");
	    	streaming_intf_detected = 1;
		}
	    else
	    {
	    	printk(KERN_ERR"- unsupported interface subclass detected, returning.\n");
	    	return retval;
	    }
	}
	else
	{
		printk(KERN_WARNING"- unsupported interface class detected: class value %x\n", intf->altsetting->desc.bInterfaceClass);
		return 0;
	}
	if (streaming_intf_detected)
	{
		// -- Create USB Dev --
		my_usb_dev = kmalloc(sizeof(struct usb_cam_dev), GFP_KERNEL);
		if (my_usb_dev == NULL)
		{
			printk(KERN_ERR"- can't allocate usb device memory.");
			return -ENOMEM;
		}
		// add ref to usb device struct count
		my_usb_dev->usb_dev = usb_get_dev(dev);
		
		// link intf with usb device
		usb_set_intfdata(intf, my_usb_dev);
		
		retval = usb_register_dev(intf, &usb_cam_class);
		if (retval < 0)
		{
			printk(KERN_ERR"- can't get minor with USBCORE\n");
			usb_set_intfdata(intf, NULL);
			return retval;
		}
		else
		{
			printk(KERN_WARNING"- registered class with USBCORE\n");
		}
		atomic_set(&my_usb_dev->urb_counter, 1);
		usb_set_interface(my_usb_dev->usb_dev, 1, 4);
		printk(KERN_WARNING"- completed probe routine");
	}
	else
	{
		retval = -ENODEV;
	}
	return retval;
}

static void usb_cam_disconnect(struct usb_interface *intf)
{
	struct usb_cam_dev *dev;
	printk(KERN_WARNING"USB CAM DISCONNECT\n");
	if (intf->altsetting->desc.bInterfaceClass == CC_VIDEO)
	{
		if (intf->altsetting->desc.bInterfaceSubClass == SC_VIDEOSTREAMING)
		{
			dev = usb_get_intfdata(intf);
			usb_set_intfdata(intf, NULL);
			usb_deregister_dev(intf, &usb_cam_class);
			kfree(dev);
			printk(KERN_WARNING"- disconnected and cleared resources\n");
		}
	}
}

static int usb_cam_open (struct inode *inode, struct file *filp)
{
	//TODO: Handle single opening if necessary
	int retval = 0;
	struct usb_interface *intf;

	printk(KERN_WARNING"USB CAM OPEN\n");
	intf = usb_find_interface(&usb_cam_driver, iminor(inode));
	if (intf == NULL)
	{
		printk(KERN_ERR"- can't find interface for usb cam driver\n");
		return -ENODEV;
	}
	filp->private_data = intf;
	return retval;
}

static int usb_cam_release (struct inode *inode, struct file *filp)
{
	int retval = 0;
	printk(KERN_WARNING"USB CAM RELEASE\n");
	//TODO: Handle single opening if necessary
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

