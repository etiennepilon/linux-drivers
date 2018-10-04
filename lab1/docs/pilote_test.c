#include <generated/autoconf.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/init.h>

#include <linux/string.h>
#include <linux/syscalls.h>
#include <linux/types.h>
#include <linux/stat.h> 

#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/cdev.h>

MODULE_AUTHOR("Bruno De Kelper");
MODULE_LICENSE("Dual BSD/GPL");

int PiloteVar = 12;
module_param(PiloteVar, int, S_IRUGO);
EXPORT_SYMBOL_GPL(PiloteVar);

//struct myModuleTag {
int  flags;
int  Tab[10];
dev_t dev;
struct class *cclass;
struct cdev  mycdev;
//} myModuleStruct;


static ssize_t module_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos);
static ssize_t module_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos);
static int module_open(struct inode *inode, struct file *filp);
static int module_release(struct inode *inode, struct file *filp);

static struct file_operations myModule_fops = {
	.owner 	 = THIS_MODULE,
	.write	 = module_write,
	.read	 = module_read,
	.open	 = module_open,
	.release = module_release,
};

static int  pilote_init (void);
static void pilote_exit (void);

static ssize_t module_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos) {
   printk(KERN_WARNING"Pilote READ : Hello, world\n");
   return 0;
}

static ssize_t module_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos) {
   printk(KERN_WARNING"Pilote WRITE : Hello, world\n");
   return 0;
}

static int module_open(struct inode *inode, struct file *filp) {
//  filp->private_data = &myModuleStruct;
  printk(KERN_WARNING"Pilote OPEN : Hello, world\n");
   return 0;
}

static int module_release(struct inode *inode, struct file *filp) {
   printk(KERN_WARNING"Pilote RELEASE : Hello, world\n");
   return 0;
}


static int __init pilote_init (void) {

   alloc_chrdev_region(&dev, 0, 1, "MyPilote");

   cclass = class_create(THIS_MODULE, "moduleTest");
   device_create(cclass, NULL, dev, NULL, "myModuleTest");

   cdev_init(&mycdev, &myModule_fops);
   cdev_add(&mycdev, dev, 1);

   printk(KERN_WARNING"Pilote : Hello, world (Pilote : %u)\n", PiloteVar);
   return 0;
}



static void __exit pilote_exit (void) {

  cdev_del(&mycdev);

  device_destroy(cclass, dev);
  class_destroy(cclass);
 
  unregister_chrdev_region(dev, 1);

  printk(KERN_ALERT"Pilote : Goodbye, cruel world\n");
}

module_init(pilote_init);
module_exit(pilote_exit);

