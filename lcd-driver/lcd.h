/*
 * lcd.h
 *
 *  Created on: April 8, 2020
 *      Author: Nick Brubaker
 */

#ifndef LCD_DRIVER_LCD_H_
#define LCD_DRIVER_LCD_H_

#define LCD_DEBUG 1  //Remove comment on this line to enable debug

#undef PDEBUG             /* undef it, just in case */
#ifdef LCD_DEBUG
#  ifdef __KERNEL__
     /* This one if debugging is on, and kernel space */
#    define PDEBUG(fmt, args...) printk( KERN_DEBUG "lcd: " fmt, ## args)
#  else
     /* This one for user space */
#    define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#  endif
#else
#  define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

struct lcd_dev
{
	struct mutex lock;
	struct cdev cdev;	  /* Char device structure		*/
};


#endif /* LCD_DRIVER_LCD_H_ */
