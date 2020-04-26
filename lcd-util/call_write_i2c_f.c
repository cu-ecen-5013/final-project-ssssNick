
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
/* Function used to read data out of shared memory and pass to LCD using the  */
/* I2C module.                                                                */
/******************************************************************************/
int call_write_i2c_f ( struct aesd_struct *util_struct )
{
    int cnt_i;
    int pid_i;

    char ln1 [16] = "From PID _____";
    char ln2 [16] = "Rec val _____";

    syslog( LOG_DEBUG, "Func 2 screen with packet @%p", util_struct );

    /* setup wiring pi */
    if (wiringPiSetup () == -1) exit (1);

    /* get file-descriptor for I2C device */
    util_struct->i2c_fd = wiringPiI2CSetup( AESD_I2C_ADDR );

    /* for lcd for 16x2 case */
    aesd_lcd_init( util_struct->i2c_fd );

    /* write default info to screen */
    aesd_lcd_clear  ( util_struct->i2c_fd );
    aesd_lcd_loc    ( util_struct->i2c_fd, util_struct->line1 );
    aesd_lcd_type_ln( util_struct->i2c_fd, "In call_write_lcd_f" );
    aesd_lcd_loc    ( util_struct->i2c_fd, util_struct->line2 );
    aesd_lcd_type_ln( util_struct->i2c_fd, "Waiting for info" );

    /* check if shared memory is set */
    if( util_struct->a == NULL )
    {
        syslog( LOG_DEBUG, "Func 2 returned since no shared memory" );
        return 0;
    }

    /* loop while shared memory exit flag clear */
    while( util_struct->a[3] == 0 )
    {
        /* if valid info in shared memory */
        /* format info and put out to lcd */
        if( util_struct->a[2] == 1)
        {
            cnt_i = util_struct->a[0];
            pid_i = util_struct->a[1];
            syslog( LOG_DEBUG, "2 Data packet size caught %d from %d", cnt_i, pid_i );

            /* convert in to string for output */
            value_pack( cnt_i, &ln2[8] );
            value_pack( pid_i, &ln1[9] );

            /* write info to lcd */
            aesd_lcd_clear  ( util_struct->i2c_fd );
            aesd_lcd_loc    ( util_struct->i2c_fd, util_struct->line1 );
            aesd_lcd_type_ln( util_struct->i2c_fd, ln1 );
            aesd_lcd_loc    ( util_struct->i2c_fd, util_struct->line2 );
            aesd_lcd_type_ln( util_struct->i2c_fd, ln2 );

            /* clear valid data flag */
            util_struct->a[2] = 0;

            /* hold data for 5 seconds before checking */
            /* and updating with new info */
            sleep(5);
        }
        else
        {
            /* if no valid data, sleep and recheck */
            sleep(1);
        }

        /* check if signal handler */
        if( util_struct->flag_exit == 1 )
        {
            break;
        }
    }

    /* allow other process to possibly get exit */
    sleep(1);

    /* write exit statement to lcd */
    aesd_lcd_clear  ( util_struct->i2c_fd );
    aesd_lcd_loc    ( util_struct->i2c_fd, util_struct->line1 );
    aesd_lcd_type_ln( util_struct->i2c_fd, "Thanks but I'm" );
    aesd_lcd_loc    ( util_struct->i2c_fd, util_struct->line2 );
    aesd_lcd_type_ln( util_struct->i2c_fd, "out of here!!" );

    /* close and clear fd */
    close(util_struct->i2c_fd);
    util_struct->i2c_fd = 0;

    return 0;
}