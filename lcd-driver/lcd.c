/**
 * @file lcd.c
 * @brief Device driver for LCD screen
 *
 * @author Nick Brubaker
 * @date 2020-4-8
 *
 * External code used: 
 * 	https://github.com/eeenjoy/I2C_LCD2004/blob/master/For_Raspberry_Pi/C/i2c_lcd2004_test.c
 *  https://gist.github.com/jnewc/f8b668c41d7d4a68f6e46f46e8c559c2
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/fs.h> // file_operations
#include <linux/delay.h>
#include <linux/i2c.h>

#define LCD_DEV_ADDR 0x27
#define CHARS_PER_ROW 20
#define NUM_ROWS 4

#define TOP_LEFT_ADDR 0x80
#define SECOND_ROW_LEFT_ADDR 0xC0
#define THIRD_ROW_LEFT_ADDR 0x94
#define BOTTOM_LEFT_ADDR 0xD4

#include "lcd.h"
int lcd_major =   0; // use dynamic major
int lcd_minor =   0;
int BLEN = 1;
size_t i2c_error;


MODULE_LICENSE("Dual BSD/GPL");

struct lcd_dev lcd_device;


/* I2C structs */

struct i2c_adapter* i2c_dev;
struct i2c_client* i2c_client;

static struct i2c_board_info __initdata board_info[] =  {
	{
		I2C_BOARD_INFO("PCF8574", LCD_DEV_ADDR),
	}
};


/* LCD helper functions */

void write_word(int data)
{
    int temp = data;
	if ( BLEN == 1 )
		temp |= 0x08;
	else
		temp &= 0xF7;
	i2c_error = i2c_smbus_write_byte(i2c_client, temp);
}

void send_command(int comm)
{
	int buf;
	// Send bit7-4 firstly
	buf = comm & 0xF0;
	buf |= 0x04;			// RS = 0, RW = 0, EN = 1
	write_word(buf);
	usleep_range(2000, 3000);
	buf &= 0xFB;			// Make EN = 0
	write_word(buf);

	// Send bit3-0 secondly
	buf = (comm & 0x0F) << 4;
	buf |= 0x04;			// RS = 0, RW = 0, EN = 1
	write_word(buf);
	usleep_range(2000, 3000);
	buf &= 0xFB;			// Make EN = 0
	write_word(buf);
}

void send_data(int data)
{
	int buf;
	// Send bit7-4 firstly
	buf = data & 0xF0;
	buf |= 0x05;			// RS = 1, RW = 0, EN = 1
	write_word(buf);
	usleep_range(2000, 3000);
	buf &= 0xFB;			// Make EN = 0
	write_word(buf);

	// Send bit3-0 secondly
	buf = (data & 0x0F) << 4;
	buf |= 0x05;			// RS = 1, RW = 0, EN = 1
	write_word(buf);
	usleep_range(2000, 3000);
	buf &= 0xFB;			// Make EN = 0
	write_word(buf);
}

void init_lcd( void )
{
	send_command(0x33);	// Must initialize to 8-line mode at first
	usleep_range(5000, 6000);
	send_command(0x32);	// Then initialize to 4-line mode
	usleep_range(5000, 6000);
	send_command(0x28);	// 2 Lines & 5*7 dots
	usleep_range(5000, 6000);
	send_command(0x0C);	// Enable display without cursor
	usleep_range(5000, 6000);
	send_command(0x01);	// Clear Screen
	i2c_smbus_write_byte(i2c_client, 0x08);
}


/* Module functions */

int lcd_open(struct inode *inode, struct file *filp)
{
	PDEBUG("open");
	return 0;
}

int lcd_release(struct inode *inode, struct file *filp)
{
	PDEBUG("release");
	return 0;
}

int fpos_to_addr(loff_t pos_in)
{
	int pos = (int)(pos_in);
	int addr = 0;

	if ((pos >= 0 * CHARS_PER_ROW) && (pos < 1 * CHARS_PER_ROW))
	{
		addr = pos - 0 * CHARS_PER_ROW + TOP_LEFT_ADDR;
	}

	else if ((pos >= 1 * CHARS_PER_ROW) && (pos < 2 * CHARS_PER_ROW))
	{
		addr = pos - 1 * CHARS_PER_ROW + SECOND_ROW_LEFT_ADDR;
	}

	else if ((pos >= 2 * CHARS_PER_ROW) && (pos < 3 * CHARS_PER_ROW))
	{
		addr = pos - 2 * CHARS_PER_ROW + THIRD_ROW_LEFT_ADDR;
	}

	else if ((pos >= 3 * CHARS_PER_ROW) && (pos < 4 * CHARS_PER_ROW))
	{
		addr = pos - 3 * CHARS_PER_ROW + BOTTOM_LEFT_ADDR;
	}

	return addr;
}

ssize_t lcd_write(struct file *filp, const char __user *buf, size_t count,
                loff_t *f_pos)
{
	ssize_t retval = -ENOMEM;
	char *kern_buf;
	size_t i;

	PDEBUG("write %zu bytes", count);
	
	mutex_lock(&(lcd_device.lock));

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
		// Check if need to start on new line
		if(*(f_pos) % 20 == 0)
		{	
			send_command(fpos_to_addr(*(f_pos)));	// Tell LCD where to write to on screen
		}
	
		send_data(kern_buf[i]);
		if(*(f_pos) < CHARS_PER_ROW * NUM_ROWS) (*(f_pos))++;
		else *(f_pos) = 0;
	}

	if(i2c_error) retval = i2c_error;
	else retval = i;

free:
	kfree(kern_buf);

out:
	mutex_unlock(&(lcd_device.lock));
	return retval;
}

loff_t lcd_llseek(struct file *filp, loff_t off, int whence)
{
	loff_t newpos;

	switch(whence) {
	  case 0: /* SEEK_SET */
		newpos = off;
		break;

	  case 1: /* SEEK_CUR */
		newpos = filp->f_pos + off;
		break;

	  case 2: /* SEEK_END */
		newpos = (CHARS_PER_ROW * NUM_ROWS) - 1;
		break;

	  default: /* can't happen */
		return -EINVAL;
	}
	if (newpos < 0) return -EINVAL;
	filp->f_pos = newpos;
	return newpos;
}

struct file_operations lcd_fops = {
	.owner =    THIS_MODULE,
	.llseek =   lcd_llseek,
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

	i2c_dev = i2c_get_adapter(1);
	i2c_client = i2c_new_device(i2c_dev, board_info);

	init_lcd();

	result = lcd_setup_cdev(&lcd_device);

	if( result ) {
		unregister_chrdev_region(dev, 1);
	}

	return result;
}

static void lcd_cleanup_module(void)
{
	dev_t devno = MKDEV(lcd_major, lcd_minor);

	cdev_del(&lcd_device.cdev);

	i2c_unregister_device(i2c_client);

	unregister_chrdev_region(devno, 1);
}

module_init(lcd_init_module);
module_exit(lcd_cleanup_module);