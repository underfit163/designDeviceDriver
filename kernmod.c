#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#define DEVICE_NAME "ttyIIVS_TAY"
#define DEVICE_CLASS "CLASS_TAY"
#define MEM_CNT 255

MODULE_LICENSE("GPL");

int init_module(void);
void cleanup_module(void);
static ssize_t readFun(struct file *, char *, size_t, loff_t *);
static ssize_t writeFun(struct file *, const char *, size_t, loff_t *);

struct file_operations fops = {
      .owner = THIS_MODULE,
      .read = readFun,
      .write = writeFun
    };
static char* my_buf;
static dev_t dev; 
static struct cdev * my_cdev;
static struct class* class_dev;
static struct device * dev_file;
static ssize_t countByte;
static int ret;
static int cd;

int init_module(void){
	printk(KERN_INFO "Hello from kernel module by Tyshkun\n");
	 /* 6.1.3*/
	my_buf = kmalloc(MEM_CNT, GFP_KERNEL);
	if(my_buf==0)
	{ 
	printk(KERN_INFO "buf error\n");
	}
/*6.1.4*/
	ret = alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME);
	if(ret<0)
	{
 	printk(KERN_INFO "alloc_chrdev_region error\n");
	return -1;
	}
	printk(KERN_INFO "Major, Minor: %d, %d\n", MAJOR(dev), MINOR(dev));
	
	 /*6.1.5*/
	my_cdev = cdev_alloc();
	if(my_cdev == NULL)
	{
	printk(KERN_INFO "cdev error\n");
	return -1;
	}
	my_cdev->owner = THIS_MODULE;
	my_cdev->ops = &fops;
	cd = cdev_add(my_cdev, dev, 1);
	if(cd < 0)
	{
 	printk(KERN_INFO "cdev_add error\n");
	return -1;
	}
 /*6.1.6*/
	class_dev = class_create(THIS_MODULE, DEVICE_CLASS);
	if(class_dev == NULL)
	{
 	printk(KERN_INFO "class_dev error\n");
	return -1;
	}
	dev_file =  device_create(class_dev, NULL, dev, NULL, DEVICE_NAME);
	if(dev_file == NULL)
	{
 	printk(KERN_INFO "dev_file error\n");
	return -1;
	}
	printk(KERN_INFO "NO ERROR INIT\n");
	return 0;
}
	
void cleanup_module(void){
	/*6.3 */
	device_destroy(class_dev, dev);
	class_destroy(class_dev);
	cdev_del(my_cdev);
	unregister_chrdev_region(dev, 1);
	kfree(my_buf);
	printk(KERN_INFO "Goodbye my friend\n");
	}

/*6.2.2*/
static ssize_t readFun(struct file *file, char __user *buf, size_t bufLen, loff_t *offset)
{
	if(*offset >= countByte)
       { printk(KERN_INFO "All read,we have nothing to read\n");
		   return 0;} 
	if(*offset + bufLen > countByte)
        bufLen = countByte - *offset;
	 if(copy_to_user(buf, my_buf + *offset, bufLen) != 0)
	 {
		printk(KERN_INFO "copy_to_user error\n");
        return -1;
	} 
	*offset += bufLen;
	printk (KERN_INFO "readFun, bufLen=%ld, pos=%d\n", bufLen, (int)*offset);
	 return bufLen;
}
	
/*6.2.1*/
static ssize_t writeFun(struct file *file, const char __user *buf, size_t bufLen, loff_t *offset)
{	
    copy_from_user(my_buf + *offset, buf, bufLen);
	*offset += bufLen;
	 countByte = bufLen;
	 printk (KERN_INFO "writeFun, bufLen=%ld, pos=%d\n", bufLen, (int)*offset);
	return bufLen;
}
