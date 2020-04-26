
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
/* Function to write PID and counter into named pipe                          */
/*                                                                            */
/* Possible that there may be many verions of this running at the same time   */
/******************************************************************************/
int call_write_pipe_f( struct aesd_struct *util_struct )
{
    syslog( LOG_DEBUG, "Write function called" );

    int val = 0;

    int ii;

    char buf[ TEST_BUFFER_SIZE ] = "my current value is _____ from _____";
  
    // FIFO file path 
    char * fifo_p = "/tmp/aesd_lcd_fifo"; 

    /* get the process id to add to packed data */
    int pid = getpid();

    /* check for seed flag and use that value */
    /* if set */
    if( util_struct->flag_s == 1 )
    {
        ii = util_struct->seed_val;
    }
    /* otherwise use 0 */
    else
    {
        ii = 0;
    }

    /* setup name pipe */
    mkfifo( fifo_p, 0666 );

    while( TRUE )
    {
        /* sleep for WR_VAL seconds */
        /* if interrupted, go back to sleep */
        /* to complete the amount of time */
        /* unless exit signaled */
        val = WR_VAL;
        while( val > 0 )
        {
            val = sleep( val );

            if( util_struct->flag_exit == 1 )
            {
                return 0;
            }
        }

        /* open pipe for writing */
        util_struct->fd = open( fifo_p, O_WRONLY | O_NONBLOCK );
        if( util_struct->fd >= 0 )
        {
            ii = (ii+5)%100000;

            /* pack values to string */
            value_pack( ii, &buf[20] );
            value_pack( pid, &buf[31] );

            /* write fixed string to pipe */
            write(  util_struct->fd, buf, 36 /*sizeof(buf)*/ );

            syslog( LOG_DEBUG, "Wrote %s", buf);

            /* close and clear pipe fd */
            close( util_struct->fd );
            util_struct->fd = 0;
        }
        else
        {
            /* if pipe not open, the check back later */
            syslog( LOG_DEBUG, "Not FIFO connect (%d); Sleep and retry", util_struct->fd );
        }

        /* check for exit from signal handler */
        if( util_struct->flag_exit == 1 )
        {
            break;
        }
    }

    return 0;
}