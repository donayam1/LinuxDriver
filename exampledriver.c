#include <linux/kernel.h>
#include <linux/init.h> 
#include <linux/module.h> //kernel module programing module_init module_exit
#include <linux/kdev_t.h> //for device file 
#include <linux/fs.h>     //file structure 
#include <linux/cdev.h>   //character device structure 
#include <linux/device.h>
#include <linux/slab.h>  //kmalloc
#include <linux/uaccess.h> //communicating with user space

#define mem_size 1024

dev_t dev=0; //holds the character major and minor number 
static struct class *dev_class;
static struct cdev my_cdev;
uint8_t *kernel_buffer;

static int __init chr_drvier_init(void);
static void __exit chr_driver_exit(void);

static int my_open (struct inode *inode, struct file *file);
static int my_release (struct inode *inode, struct file *file);
static ssize_t my_read (struct file *file, char __user *buf, size_t len, loff_t *loff);
static ssize_t my_write (struct file *file, const char __user *user, size_t, loff_t *loff);


static struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = my_read,
	.write =my_write,
	.open =my_open,
	.release=my_release,
};

static int my_open(struct inode *inode, struct file *file){
	/*Creating Physical Memory */
	if((kernel_buffer=kmalloc(mem_size,GFP_KERNEL)) == 0){
		printk(KERN_INFO "Can not allocate kernel buffer\n");
		return -1;
	}
	printk(KERN_INFO "device file opened. \n");
	return 0;
}

static int my_release (struct inode *inode, struct file *file){
	kfree(kernel_buffer);
	printk(KERN_INFO "Device file closed \n");
	return 0;
}
static ssize_t my_read (struct file *file, char __user *buf, size_t len, loff_t *loff){
	copy_to_user(buf,kernel_buffer,len);
	printk(KERN_INFO "Data read: done\n");
	return len;
}
static ssize_t my_write (struct file *file, const char __user *buf, size_t len, loff_t *loff)
{
	copy_from_user(kernel_buffer,buf,len);
	printk(KERN_INFO "Data is written successfully. \n");
	return len;
}

static int __init chr_drvier_init(void){
	/*Allocate Major number*/
	if((alloc_chrdev_region(&dev,0,1,"my_Dev"))<0){
		printk(KERN_INFO "Can not allocate module major number.\n");
		return -1;
	}
	printk(KERN_INFO "Major=%d Minor=%d ",MAJOR(dev),MINOR(dev));
	/*creating cdev structure*/
	cdev_init(&my_cdev,&fops);
	
	/* adding character device to the system*/
	if((cdev_add(&my_cdev,dev,1)<0)){
		printk(KERN_INFO "Can not add device to the system");
		goto r_class;
	}

	/*creating struct class*/
	if((dev_class = class_create(THIS_MODULE,"my_class0")) == NULL){
		printk(KERN_INFO "Unable to create the struct class.\n");
		goto r_class;
	}

	/* Creating device */
	if((device_create(dev_class,NULL,dev,NULL,"my_device")) == NULL){
		printk(KERN_INFO "Can not create device\n");
		goto r_device;
	}

	printk(KERN_INFO "device driver installed properly.\n");
	return 0;
r_device:
	class_destroy(dev_class);
	
r_class:
	unregister_chrdev_region(dev,1);
	return -1;


}
static void __exit chr_driver_exit(void){
	device_destroy(dev_class,dev);
	class_destroy(dev_class);
	cdev_del(&my_cdev);
	unregister_chrdev_region(dev,1);
	printk(KERN_INFO "Device driver is removed sucesfully.");
}



module_init(chr_drvier_init);
module_exit(chr_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Donayam");
//MODULE_DESCRIPTION("Character driver exersice.");


