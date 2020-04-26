
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
/* Function to parse the openvpn log to see if there are any connections      */
/******************************************************************************/
int call_read_log_f ( struct aesd_struct *util_struct )
{
    FILE *log_fd;

    int ii = 0;

    int log_found = 0;

    char buffer[BUFFER_SIZE];
    char ipaddr[16] = "                ";
    char emptys[16] = "                ";

    char cmp_str[11] = "vpn.server,";

    //syslog( LOG_DEBUG, "In call read log" );

    /* open openvp status log to parse */
    log_fd = fopen( "/var/log/openvpn-status.log", "r" );

    /* if log doesn't exit or failed to open */
    /* exit: calling process will probably try */
    /* again later */
    if( log_fd <= 0 )
    {
        syslog( LOG_ERR, "ERROR - OpenVPN status log cannot be opened (%d)", errno );
        util_struct->log_found = 0;
        return -1;
    }

    //syslog( LOG_DEBUG, "OpenVPN log file opened @ %p", log_fd );

    /* read file until end */
    while( fgets( buffer, sizeof(buffer), log_fd ) )
    {
        //syslog( LOG_DEBUG, "OpenVPN log: %s", buffer );

        /* check for specific format */
        for( ii=0; ii<11; ii++ )
        {
            if( buffer[ii] != cmp_str[ii] )
            {   
                //syslog( LOG_DEBUG, "OpenVPN log no match (%d)", ii );
                goto no_match;
            }
        }
        /* after finding the ref; copy until ':' */
        for( ii=11; ii<27; ii++ )
        {
            if( buffer[ii] == ':' )
            {
                goto match;
            }

            ipaddr[ii-11] = buffer[ii];
        }
match:
        /* flag that address was found and break from loop */
        log_found = 1;
        //syslog( LOG_DEBUG, "OpenVPN log set value with %s (%d)", ipaddr, ii );
        break;
no_match:
        /* not found in log */
        log_found = 0;
        //syslog( LOG_DEBUG, "continue" );
    }

    /* close log */
    fclose( log_fd );

    /* if found, store to structure and flag */
    if( log_found == 1 )
    {
        strcpy( util_struct->log_ip_addr, ipaddr );
        util_struct->log_found = 1;
        //syslog( LOG_DEBUG, "Log found set");
    }
    /* else copy empty char and clear found flag */
    else
    {
        strcpy( util_struct->log_ip_addr, emptys );
        util_struct->log_found = 0;
        //syslog( LOG_DEBUG, "Log found cleared");
    }

    return 0;
}