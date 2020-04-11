/**
 * @file lcd.c
 * @brief Device driver for LCD screen
 *
 * @author Nick Brubaker
 * @date 2020-4-8
 *
 * External code used: 
 * 	https://github.com/eeenjoy/I2C_LCD2004/blob/master/For_Raspberry_Pi/C/i2c_lcd2004_test.c
 *  https://github.com/WiringPi/WiringPi/blob/master/wiringPi/wiringPiI2C.c
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/fs.h> // file_operations
#include <linux/i2c.h>

#include "lcd.h"
int lcd_major =   0; // use dynamic major
int lcd_minor =   0;
int LCDAddr = 0x3F;
int fd;

MODULE_LICENSE("Dual BSD/GPL");

struct lcd_dev lcd_device;

int lcd_open(struct inode *inode, struct file *filp)
{
	struct lcd_dev *dev;

	PDEBUG("open");

	dev = container_of(inode->i_cdev, struct lcd_dev, cdev);
	filp->private_data = dev;

	return 0;
}

int lcd_release(struct inode *inode, struct file *filp)
{
	PDEBUG("release");
	return 0;
}

ssize_t lcd_write(struct file *filp, const char __user *buf, size_t count,
                loff_t *f_pos)
{
	ssize_t retval = -ENOMEM;
	struct lcd_dev *dev = filp->private_data;
	char *kern_buf;
	size_t i;

	PDEBUG("write %zu bytes",count);
	
	mutex_lock(&(dev->lock));

	kern_buf = kmalloc(count, GFP_KERNEL);
	if(kern_buf == NULL)
	{
		goto out;
	}

	if(copy_from_user(kern_buf, buf, count))
	{
		retval = -EFAULT;
		goto free;
	}

	for(i = 0; i < count; i++)
	{
		
	}

	retval = count;

free:
	kfree(kern_buf);

out:
	mutex_unlock(&(dev->lock));
	return retval;
}

struct file_operations lcd_fops = {
	.owner =    THIS_MODULE,
	.write =    lcd_write,
};

static int lcd_setup_cdev(struct lcd_dev *dev)
{
	int err, devno = MKDEV(lcd_major, lcd_minor);

	cdev_init(&dev->cdev, &lcd_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &lcd_fops;
	err = cdev_add (&dev->cdev, devno, 1);
	if (err) {
		printk(KERN_ERR "Error %d adding lcd cdev", err);
	}
	return err;
}

static int lcd_init_module(void)
{	
	dev_t dev = 0;
	int result;
	result = alloc_chrdev_region(&dev, lcd_minor, 1,
			"lcd");
	lcd_major = MAJOR(dev);
	if (result < 0) {
		printk(KERN_WARNING "Can't get major %d\n", lcd_major);
		return result;
	}
	memset(&lcd_device,0,sizeof(struct lcd_dev));

	mutex_init(&(lcd_device.lock));

	result = lcd_setup_cdev(&lcd_device);

	if( result ) {
		unregister_chrdev_region(dev, 1);
	}

	if ((fd = open("/dev/i2c-0", O_RDWR)) < 0)
	//init();

	return result;
}

static void lcd_cleanup_module(void)
{
	dev_t devno = MKDEV(lcd_major, lcd_minor);

	cdev_del(&lcd_device.cdev);

	unregister_chrdev_region(devno, 1);
}

module_init(lcd_init_module);
module_exit(lcd_cleanup_module);