
#include "aesd_lcd_util.h"

/******************************************************************************/
/* Code Ref                                                                   */
/*
 * wiringPi:
 *  Arduino look-a-like Wiring library for the Raspberry Pi
 *  Copyright (c) 2012-2017 Gordon Henderson
 *  Additional code for pwmSetClock by Chris Hall <chris@kchall.plus.com>
 *
 *  Thanks to code samples from Gert Jan van Loo and the
 *  BCM2835 ARM Peripherals manual, however it's missing
 *  the clock section /grr/mutter/
 ***********************************************************************
 * This file is part of wiringPi:
 *  https://projects.drogon.net/raspberry-pi/wiringpi/
 *
 *    wiringPi is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as
 *    published by the Free Software Foundation, either version 3 of the
 *    License, or (at your option) any later version.
 *
 *    wiringPi is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with wiringPi.
 *    If not, see <http://www.gnu.org/licenses/>.
 ***********************************************************************/
/******************************************************************************/
/* Additional reference used:                                                 */
/* https://stackoverflow.com/questions/1570511/c-code-to-get-the-ip-address   */
/******************************************************************************/

/******************************************************************************/
/* re-purposed functions from the lcd example                                 */
/* in wiringPi                                                                */
/******************************************************************************/

/******************************************************************************/
/* Breaks info to write into upper/lower bytes and call write to i2c lcd      */
/******************************************************************************/
void aesd_lcd_byte( int input_fd, int bits, int mode )
{
    //Send byte to data pins
    // bits = the data
    // mode = 1 for data, 0 for command
    int bits_high;
    int bits_low;
    // uses the two half byte writes to LCD
    bits_high = mode | (bits & 0xF0) | AESD_LCD_BACKLT;
    bits_low  = mode | ((bits << 4) & 0xF0) | AESD_LCD_BACKLT;

    // High bits
    wiringPiI2CReadReg8(input_fd, bits_high);
    aesd_lcd_toggle(input_fd, bits_high);

    // Low bits
    wiringPiI2CReadReg8(input_fd, bits_low);
    aesd_lcd_toggle(input_fd, bits_low);
}

/******************************************************************************/
/* Clear the screen                                                           */
/******************************************************************************/
void aesd_lcd_clear( int input_fd )
{
    aesd_lcd_byte( input_fd, 0x01, AESD_LCD_CMD );
    aesd_lcd_byte( input_fd, 0x02, AESD_LCD_CMD );
}

/******************************************************************************/
/* Setup LCD for writing 16x2 using I2C Wiring Pi source code                 */
/******************************************************************************/
void aesd_lcd_init( int input_fd )
{
    // Initialise display
    aesd_lcd_byte( input_fd, 0x33, AESD_LCD_CMD ); // Initialise
    aesd_lcd_byte( input_fd, 0x32, AESD_LCD_CMD ); // Initialise
    aesd_lcd_byte( input_fd, 0x06, AESD_LCD_CMD ); // Cursor move direction
    aesd_lcd_byte( input_fd, 0x0C, AESD_LCD_CMD ); // 0x0F On, Blink Off
    aesd_lcd_byte( input_fd, 0x28, AESD_LCD_CMD ); // Data length, number of lines, font size
    aesd_lcd_byte( input_fd, 0x01, AESD_LCD_CMD ); // Clear display
    delayMicroseconds(500);
}

/******************************************************************************/
/* Seek to address location                                                   */
/******************************************************************************/
void aesd_lcd_loc( int input_fd, int line )
{
    aesd_lcd_byte( input_fd, line, AESD_LCD_CMD );
}

/******************************************************************************/
/* Toggle enable pin on LCD display                                           */
/******************************************************************************/
void aesd_lcd_toggle( int input_fd, int bits )
{
    delayMicroseconds(500);
    wiringPiI2CReadReg8(input_fd, (bits | AESD_LCD_ENABLE));
    delayMicroseconds(500);
    wiringPiI2CReadReg8(input_fd, (bits & ~AESD_LCD_ENABLE));
    delayMicroseconds(500);
}

/******************************************************************************/
/* Write line to lcd                                                          */
/******************************************************************************/
void aesd_lcd_type_ln( int input_fd, const char *s )
{
    while( *s )
    {
        aesd_lcd_byte( input_fd, *(s++), AESD_LCD_CHR);
    }
}

/******************************************************************************/
/* Function to call as default for unused function calls                      */
/******************************************************************************/
int call_default_f( struct aesd_struct *util_struct )
{
    syslog( LOG_DEBUG, "Default function called" );

    syslog( LOG_DEBUG, " - Not currently setup to run any additional functionality" );

    return 0;
}

/******************************************************************************/
/* Function to clear any allocated memory or close any open file descriptors  */
/******************************************************************************/
void clean_up_f( struct aesd_struct *util_struct )
{
    struct aesd_ll_struct *walker = NULL;

    syslog( LOG_DEBUG, "Cleaning up aesd util" );

    /* clear linked-list */
    while( util_struct->head != NULL )
    {
        walker = util_struct->head;
        util_struct->head = util_struct->head->nxptr;
        free(walker);

        syslog( LOG_DEBUG, "Freeing ll memory" );
    }

    /* clear handle to i2c module */
    if( util_struct->i2c_fd > 0 )
    {
        syslog( LOG_DEBUG, "Closing I2C FD" ); 
        close( util_struct->i2c_fd );
        util_struct->i2c_fd = 0;
    }

    /* clear handle to lcd module */
    if( util_struct->lcd_fd > 0 )
    {
        syslog( LOG_DEBUG, "Closing LCD FD" ); 
        close( util_struct->lcd_fd );
        util_struct->lcd_fd = 0;
    }

    syslog( LOG_DEBUG, "Closing log" );    

    /* close out syslogging */
    closelog();
}

/******************************************************************************/
/* Signal handler used in all modes                                           */
/******************************************************************************/
void common_sig_handle_f( int sig )
{
    syslog( LOG_DEBUG, "Sig %d caught", sig );

    *flag_to_exit = 1;
}

/******************************************************************************/
/* Function to test writing to lcd using I2C module                           */
/******************************************************************************/
int test_func_1( struct aesd_struct *util_struct )
{
    int fd1 = 0;

    syslog( LOG_DEBUG, "Test func 1; testing LCD with ported functions" );

    if (wiringPiSetup () == -1) exit (1);

    fd1 = wiringPiI2CSetup( AESD_I2C_ADDR );

    // lcd_init(); // setup LCD
    aesd_lcd_init( fd1 );

    aesd_lcd_clear  ( fd1 );
    aesd_lcd_loc    ( fd1, util_struct->line1 );
    aesd_lcd_type_ln( fd1, "Thanks for the" );
    aesd_lcd_loc    ( fd1, util_struct->line2 );
    aesd_lcd_type_ln( fd1, "time we shared" );

    close(fd1);

    fd1 = 0;

    return 0;
}

/******************************************************************************/
/* Light weight function to convert 5 digit integer value to char string      */
/******************************************************************************/
void value_pack( int input_val, char *buf )
{
    buf[4] = 48 + ((input_val/1)%10);
    buf[3] = 48 + ((input_val/10)%10);
    buf[2] = 48 + ((input_val/100)%10);
    buf[1] = 48 + ((input_val/1000)%10);
    buf[0] = 48 + ((input_val/10000)%10);
}

/******************************************************************************/
/* Light weight function to convert 5 digit char string to integer            */
/******************************************************************************/
int value_unpack( char *buf )
{
    int val = 0;

    val += 1*(buf[4]-48);
    val += 10*(buf[3]-48);
    val += 100*(buf[2]-48);
    val += 1000*(buf[1]-48);
    val += 10000*(buf[0]-48);

    return val;
}