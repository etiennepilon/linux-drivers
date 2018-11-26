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
	.name       = "usbcam",
	.id_table   = usb_cam_table,
	.probe      = usb_cam_probe,
	.disconnect = usb_cam_disconnect,
};

// class
static struct usb_class_driver usb_cam_class = {
	.name       = "usb/usbcam%d",
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
	//TODO: Handle single opening + O_RDONLY flag if necessary
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
	int retval = 0, user_value = 0;
	struct usb_interface *intf = filp->private_data;
    struct usb_cam_dev *cam = usb_get_intfdata(intf);
    int direction = 0;
    char cam_position[4] = {0};
    char set_data[2] = {0};
    char pantilt_reset_value = 0x03;

    printk(KERN_WARNING"USB CAM IOCTL CALL\n");
    
    if(_IOC_TYPE(cmd) != USB_CAM_IOC_MAGIC || _IOC_NR(cmd) > USB_CAM_IOC_MAXNR)
    {
    	printk(KERN_ERR"- Error: received invalid IOCTL command\n");
    	return -ENOTTY;
    }
    
    if(_IOC_DIR(cmd) & _IOC_WRITE)
    	retval =  !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
    else if(_IOC_DIR(cmd) & _IOC_READ)
    	retval = !access_ok(VERIFY_WRITE, (void __user*)arg, _IOC_SIZE(cmd));
    if(retval)
    {
    	printk(KERN_ERR"- Error: Can't access user variables\n");
        return -EFAULT;
    }

    
    switch(cmd)
    {
        case USB_CAM_IOCTL_STREAMON:
            printk(KERN_WARNING"- STREAMON command received\n");
            retval = usb_control_msg(
                cam->usb_dev,
                usb_sndctrlpipe(cam->usb_dev, cam->usb_dev->ep0.desc.bEndpointAddress),
                0x0B,
                (USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_INTERFACE),
                0x0004,
                0x0001,
                NULL,
                0,
                0);
            if (retval < 0)
            {
                printk(KERN_ERR"- Error sending control msg in STREAMON cmd\n");
                return retval;
            }
        break;
        case USB_CAM_IOCTL_STREAMOFF:
            printk(KERN_WARNING"- STREAMOFF command received\n");
            retval = usb_control_msg(
                cam->usb_dev,
                usb_sndctrlpipe(cam->usb_dev, cam->usb_dev->ep0.desc.bEndpointAddress),
                0x0B,
                (USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_INTERFACE),
                0x0000,
                0x0001,
                NULL,
                0,
                0);
            if (retval < 0)
            {
                printk(KERN_ERR"- Error sending control msg in STREAMOFF cmd\n");
                return retval;
            }
        break;
        case USB_CAM_IOCTL_PANTILT:

            printk(KERN_WARNING"- PANTILT command received\n");
            
            retval = __get_user(direction, (unsigned int __user *) arg);
            if (retval < 0)
            {
            	printk(KERN_WARNING"- Error getting user direction value\n");
            }
            switch(direction)
            {
                case 0: //UP
                    printk(KERN_WARNING"- Going up\n");
                    cam_position[0] = 0x00;
                    cam_position[1] = 0x00;
                    cam_position[2] = 0x80;
                    cam_position[3] = 0xFF;
                break;
                case 1: //DOWN
                    printk(KERN_WARNING"- Going down\n");
                    cam_position[0] = 0x00;
                    cam_position[1] = 0x00;
                    cam_position[2] = 0x80;
                    cam_position[3] = 0x00;
                break;
                case 2: //LEFT
                    printk(KERN_WARNING"- Going left\n");
                    cam_position[0] = 0x80;
                    cam_position[1] = 0x00;
                    cam_position[2] = 0x00;
                    cam_position[3] = 0x00;
                break;
                case 3: //RIGHT
                    printk(KERN_WARNING"- Going right\n");
                    cam_position[0] = 0x80;
                    cam_position[1] = 0xFF;
                    cam_position[2] = 0x00;
                    cam_position[3] = 0x00;
                break;
                default:
                    printk(KERN_ERR"- received invalid command\n");
                    return -1;
                break;
            }
            retval = usb_control_msg(
                cam->usb_dev,
                usb_sndctrlpipe(cam->usb_dev, cam->usb_dev->ep0.desc.bEndpointAddress),
                0x01,
                (USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_INTERFACE),
                0x0100,
                0x0900,
                &cam_position,
                4,
                0);
            if (retval < 0)
            {
                printk(KERN_ERR"- Error sending control msg in PANTILT cmd\n");
                return retval;
            }
            
        break;
        case USB_CAM_IOCTL_PANTILT_RESET:
            printk(KERN_WARNING"- PANTILT_RESET command received\n");
            
            retval = usb_control_msg(
                cam->usb_dev,
                usb_sndctrlpipe(cam->usb_dev, cam->usb_dev->ep0.desc.bEndpointAddress),
                0x01,
                (USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_INTERFACE),
                0x0200,
                0x0900,
                &pantilt_reset_value,
                1,
                0);
            if (retval < 0)
            {
                printk(KERN_ERR"- Error sending control msg in PANTILT_RESET cmd\n");
                return retval;
            }
            
        break;
        case USB_CAM_IOCTL_SET:
            printk(KERN_WARNING"- SET command received\n");
            //TODO: Retrieve index, value, and data from user program.
            // INCOMPLETE
            retval = __get_user(user_value, (unsigned int __user *) arg);
            if (retval < 0)
            {
            	printk(KERN_WARNING"- Error getting user direction value\n");
            }
            set_data[0] = user_value & 0xFF;
            set_data[1] = (user_value >> 8) & 0xFF; 
            // TODO: Cast into two chars the user value
            retval = usb_control_msg(
                cam->usb_dev,
                usb_sndctrlpipe(cam->usb_dev, cam->usb_dev->ep0.desc.bEndpointAddress),
                0x01,
                (USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE),
                0, // TODO: WTF is this
                0x0200,
				set_data,
                2,
                0);
            if (retval < 0)
            {
                printk(KERN_ERR"- Error sending control msg in PANTILT_RESET cmd\n");
                return retval;
            }
            
        break;
        /*
        case USB_CAM_IOCTL_GET:
            unsigned char request = 0;

            printk(KERN_WARNING"- GET command received\n");
            __get_user(request, (unsigned char __user *) arg);
            if (request != GET_CUR && request != GET_MIN && request != GET_MAX && request != GET_RES)
            {
                printk(KERN_WARNING"- Invalid command in GET\n");
                return -EINVAL;
            }
            retval = usb_control_msg(
                cam->usb_dev,
                usb_rcvctrlpipe(cam->usb_dev, cam->usb_dev->ep0.desc.bEndpointAddress),
                request,
                (USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE),
                0,
                0x0200,
                NULL,
                2,
                0);
            if (retval < 0)
            {
                printk(KERN_ERR"- Error sending control msg in PANTILT_RESET cmd\n");
                return retval;
            }

        break;
        */
        default:
            printk(KERN_WARNING"- Received invalid command\n");
            return -EINVAL;
        break;
    }
    

    return retval;
}

