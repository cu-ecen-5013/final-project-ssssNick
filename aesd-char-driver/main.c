/**
 * @file aesdchar.c
 * @brief Functions and data related to the AESD char driver implementation
 *
 * Based on the implementation of the "scull" device driver, found in
 * Linux Device Drivers example code.
 *
 * @author Dan Walkes
 * @date 2019-10-22
 * @copyright Copyright (c) 2019
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include<linux/slab.h>
#include <linux/uaccess.h>
#include <linux/fs.h> // file_operations
#include "aesd-circular-buffer.h"
#include "aesdchar.h"
int aesd_major =   0; // use dynamic major
int aesd_minor =   0;

MODULE_AUTHOR("Nick Brubaker");
MODULE_LICENSE("Dual BSD/GPL");

struct aesd_dev aesd_device;

int aesd_open(struct inode *inode, struct file *filp)
{
	struct aesd_dev *dev;

	PDEBUG("open");

	dev = container_of(inode->i_cdev, struct aesd_dev, cdev);
	filp->private_data = dev;

	return 0;
}

int aesd_release(struct inode *inode, struct file *filp)
{
	PDEBUG("release");
	return 0;
}

ssize_t aesd_read(struct file *filp, char __user *buf, size_t count,
                loff_t *f_pos)
{
	ssize_t retval = 0;
	struct aesd_dev *dev = filp->private_data;
	size_t i = 0;
	size_t bytes_to_read = 0;
	size_t command_remaining_bytes = 0;
	size_t entry_offset;
	struct aesd_buffer_entry *current_entry;
	char *kern_buf;

	PDEBUG("read %zu bytes with offset %lld",count,*f_pos);
	
	mutex_lock(&(dev->lock));
	
	current_entry = aesd_circular_buffer_find_entry_offset_for_fpos(&(dev->circ_buf),
			*f_pos, &entry_offset);
	if(current_entry == NULL)
	{
		(*f_pos) = 0;
		goto out;	//	Nothing left to read
	}

	command_remaining_bytes = (current_entry->size - entry_offset);
	if(command_remaining_bytes < count) bytes_to_read = command_remaining_bytes;
	else bytes_to_read = count;

	kern_buf = kmalloc(bytes_to_read ,GFP_KERNEL);
	if(kern_buf == NULL)
	{
		retval = -ENOMEM;
		goto out;
	}

	// Continue until count bytes read or end of command reached
	for(i = 0; i < bytes_to_read; i++, entry_offset++)	
	{
		kern_buf[i] = current_entry->buffptr[entry_offset];
	}	
	(*f_pos) += i;
	retval = i;

	if(copy_to_user(buf, kern_buf, i) != 0)
	{
		retval = -EFAULT;
	}

	kfree(kern_buf);

out:
	mutex_unlock(&(dev->lock));
	return retval;
}

ssize_t aesd_write(struct file *filp, const char __user *buf, size_t count,
                loff_t *f_pos)
{
	ssize_t retval = -ENOMEM;
	struct aesd_dev *dev = filp->private_data;
	const char *add_result;
	char *kern_buf;
	struct aesd_buffer_entry new_entry;
	size_t i;

	PDEBUG("write %zu bytes with offset %lld",count,*f_pos);
	
	mutex_lock(&(dev->lock));

	kern_buf = kmalloc(count, GFP_KERNEL);
	if(kern_buf == NULL)
	{
		goto out;
	}

	dev->working_entry_buf = krealloc(dev->working_entry_buf, count, GFP_KERNEL);
	if(dev->working_entry_buf == NULL)
	{
		goto free_kbuf;
	}

	if(copy_from_user(kern_buf, buf, count))
	{
		retval = -EFAULT;
		goto free;
	}

	for(i = 0; i < count; i++, (dev->working_entry_size)++)
	{
		(dev->working_entry_buf)[dev->working_entry_size] = kern_buf[i];
	}

	retval = count;

	if((dev->working_entry_buf)[(dev->working_entry_size - 1)] == '\n')
	{
		new_entry.buffptr = dev->working_entry_buf;
		new_entry.size = dev->working_entry_size;

		add_result = aesd_circular_buffer_add_entry(&(dev->circ_buf), &(new_entry));
		if(add_result != NULL) kfree(add_result);
		dev->working_entry_size = 0;

free:		
		dev->working_entry_buf = NULL;
	}

free_kbuf:
	kfree(kern_buf);

out:
	mutex_unlock(&(dev->lock));
	return retval;
}

loff_t aesd_llseek(struct file *filp, loff_t off, int whence)
{
	struct aesd_dev *dev = filp->private_data;
	loff_t newpos;

	switch(whence) {
	  case 0: /* SEEK_SET */
		newpos = off;
		break;

	  case 1: /* SEEK_CUR */
		newpos = filp->f_pos + off;
		break;

	  case 2: /* SEEK_END */
		newpos = aesd_circular_buffer_find_end(&(dev->circ_buf));
		break;

	  default: /* can't happen */
		return -EINVAL;
	}
	if (newpos < 0) return -EINVAL;
	filp->f_pos = newpos;
	return newpos;
}

struct file_operations aesd_fops = {
	.owner =    THIS_MODULE,
	.llseek =   aesd_llseek,
	.read =     aesd_read,
	.write =    aesd_write,
	.open =     aesd_open,
	.release =  aesd_release,
};

static int aesd_setup_cdev(struct aesd_dev *dev)
{
	int err, devno = MKDEV(aesd_major, aesd_minor);

	cdev_init(&dev->cdev, &aesd_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &aesd_fops;
	err = cdev_add (&dev->cdev, devno, 1);
	if (err) {
		printk(KERN_ERR "Error %d adding aesd cdev", err);
	}
	return err;
}



int aesd_init_module(void)
{
	dev_t dev = 0;
	int result;
	result = alloc_chrdev_region(&dev, aesd_minor, 1,
			"aesdchar");
	aesd_major = MAJOR(dev);
	if (result < 0) {
		printk(KERN_WARNING "Can't get major %d\n", aesd_major);
		return result;
	}
	memset(&aesd_device,0,sizeof(struct aesd_dev));

	mutex_init(&(aesd_device.lock));

	result = aesd_setup_cdev(&aesd_device);

	if( result ) {
		unregister_chrdev_region(dev, 1);
	}
	return result;

}

void aesd_cleanup_module(void)
{
	struct aesd_buffer_entry *entryptr;
	uint8_t index;

	dev_t devno = MKDEV(aesd_major, aesd_minor);

	cdev_del(&aesd_device.cdev);

	AESD_CIRCULAR_BUFFER_FOREACH(entryptr,&(aesd_device.circ_buf),index)
	{
		kfree(entryptr->buffptr);
	}

	unregister_chrdev_region(devno, 1);
}



module_init(aesd_init_module);
module_exit(aesd_cleanup_module);
