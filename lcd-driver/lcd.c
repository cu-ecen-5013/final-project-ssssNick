/**
 * @file lcd.c
 * @brief Device driver for LCD screen
 *
 * @author Nick Brubaker
 * @date 2020-4-8
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/fs.h> // file_operations
#include "lcd.h"
int lcd_major =   0; // use dynamic major
int lcd_minor =   0;

MODULE_LICENSE("Dual BSD/GPL");

struct lcd_dev lcd_device;

struct file_operations lcd_fops = {
	.owner =    THIS_MODULE
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

	printk(KERN_ALERT "Hello from LCD module!\n");

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