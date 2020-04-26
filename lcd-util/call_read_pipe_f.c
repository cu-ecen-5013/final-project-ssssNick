
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
/* Function used to read information out of named pipe as it detects it has   */
/* any and then send out over shared memory.                                  */
/******************************************************************************/
int call_read_pipe_f( struct aesd_struct *util_struct )
{
    syslog( LOG_DEBUG, "Read test function called" );

    int rd_sz = 0;

    int rc;

    int cnt_i;
    int pid_i;
    int local_exit = 0;

    char buf[ TEST_BUFFER_SIZE ];

    char * fifo_p = "/tmp/aesd_lcd_fifo";

    /* polling structure to detect event */
    struct pollfd poll_s;

    /* setup named pipe */
    mkfifo( fifo_p, 0666 );

    /* open handle to named pipe for non-blocking read mode */
    util_struct->fd = open( fifo_p, O_RDONLY | O_NONBLOCK );

    /* pass handle for using polling mode */
    poll_s.fd     = util_struct->fd;
    poll_s.events = POLLIN;

    /* loop until exit signal from signal handler or */
    /* from the shared memory exit signal */
    while( TRUE )
    {
        /* poll pipe or timeout after 1 sec */
        rc = poll( &poll_s, 1, RD_VAL );

        /* if polling detected event; check event for errors */
        if( (rc == 1) && ((poll_s.revents & 0x0001) == POLLIN) && (poll_s.revents < 32))
        {
            /* detected valid data to read */
            rd_sz = read( util_struct->fd, buf, 36/*sizeof(buf)*/ );

            /* end rec string with null */
            if( (rd_sz < TEST_BUFFER_SIZE) )
            {
                buf[rd_sz] = '\0';
            }

            /* convert strings from data to integers */
            cnt_i = value_unpack( &buf[20] );
            pid_i = value_unpack( &buf[31] );

            syslog( LOG_DEBUG, "Read rec %s; (%d bytes, %d, %d)", 
                buf, 
                rd_sz, 
                cnt_i,
                pid_i
            );

            /* check if shared memory setup */
            if( util_struct->a != NULL )
            {
                /* wait for "mutex" to be able to add data to shared memory */
                /* single int used as mutex and not actual mutex used */
                /* exit from blocking on either exit cond */
                while( (util_struct->a[2] == 1) && (util_struct->a[3] == 0) && (util_struct->flag_exit == 0) )
                {
                    sleep(1);
                }

                /* add data to shared mem */
                util_struct->a[0] = cnt_i;
                util_struct->a[1] = pid_i;

                /* signal valid data in shared mem */
                util_struct->a[2] = 1;

                if( util_struct->a[3] == 1 )
                {
                    local_exit = 1;
                }
            }
        }
        /* timeout detected */
        else if( rc == 0 )
        {
            syslog( LOG_DEBUG, "Read timeout" );
        }
        /* there several valid event that can happen */ 
        /* don't need to worry about them for this instance */
        else
        {
            // syslog( LOG_DEBUG, "Read error with rc %d w/ revents %d, events %d, (%d)", rc, poll_s.revents, poll_s.events, errno );
            sleep( ONE );
        }

        /* check for shared memory exit condition */
        if( util_struct->a != NULL )
        {
            if( util_struct->a[3] == 1 )
            {
                local_exit = 1;
            }
        }

        /* check for signal handler exit condition */
        if( (util_struct->flag_exit == 1) || (local_exit == 1) )
        {
            close( util_struct->fd );
            break;
        }
    }

    return 0;
}