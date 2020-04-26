
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
/* Function used for testing                                                  */
/*                                                                            */
/* Function will read shared memory values to syslog                          */
/******************************************************************************/
int rd_shared_mem_f( struct aesd_struct *util_struct )
{
    int cnt_i;
    int pid_i;
    syslog( LOG_DEBUG, "Func 2 screen with packet @%p", util_struct );

    if( util_struct->a == NULL )
    {
        syslog( LOG_DEBUG, "Func 2 returned since no shared memory" );
        return 0;
    }

    while( util_struct->a[3] == 0 )
    {
        if( util_struct->a[2] == 1)
        {
            cnt_i = util_struct->a[0];
            pid_i = util_struct->a[1];
            syslog( LOG_DEBUG, "2 Data packet size caught %d from %d", cnt_i, pid_i );
            util_struct->a[2] = 0;
        }
        else
        {
            sleep(1);
        }

        if( util_struct->flag_exit == 1 )
        {
            break;
        }
    }
    sleep(1);

    return 0;
}