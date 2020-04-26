
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
/* Locally used functions                                                     */
/******************************************************************************/
int write_lcd_line_f ( int fd, int line_num, char str[20] );
int collect_ip_addr_f( char ip_addr[17] );

/******************************************************************************/
/* Function to write formatted data to LCD using LCD Module                   */
/******************************************************************************/
int call_write_lcd_f ( struct aesd_struct *util_struct )
{
    int ii,iii;

    int ptr_cnt = 0;
    int fd = 0;

    /* for finding system readable time */
    time_t rawtime;
    struct tm *current;
    char *ctm;

    /* template for each lcd line */
    char ip_addr [17] = "                 ";
    char no_addr [20] = "VPN No connection/s ";
    char unknw [7]    = "Unknown";

    char ln1 [20] = "                    ";
    char ln2 [20] = "IP                  ";
    char ln3 [20] = "VPN No connection/s ";
    char ln4 [60] = ">  Ayden Blotnick  <>  Nick Brubaker   <> Martin Lennartz  <";

    syslog( LOG_DEBUG, "call_write_lcd_f" );

    /* find address of current device */
    /* store to string */
    if( collect_ip_addr_f( ip_addr ) > 0 )
    {
        for( ii=0; ii<17; ii++ )
        {
            if( ip_addr[ii] == '\0' )
            {
                break;
            }
            ln2[ii+3] = ip_addr[ii];
        }
    }
    else
    {
        for( ii=0; ii<7; ii++ )
        {
            ln2[ii+3] = unknw[ii];
        }
    }

    /* store local file descriptor from struct */
    fd = util_struct->lcd_fd;
    
    /* check for validity */
    if( fd < 0 )
    {
        syslog( LOG_ERR, "ERROR - Failed to open LCD file descriptor with returned fd of %d (%d)", fd , errno );
        return -1;
    }

    syslog( LOG_DEBUG, "LCD opened" );

    /* clear screen using LCD module ioctl command */
    ioctl(fd, LCD_IOCLEARSCREEN, 0);

    syslog( LOG_DEBUG, "LCD cleared" );

    sleep(1);

    /* write opening statement to screen */
    write_lcd_line_f( fd, 1, "Setting everything  ");
    write_lcd_line_f( fd, 2, "up so we can see wha");
    write_lcd_line_f( fd, 3, "t is going to happen");
    write_lcd_line_f( fd, 4, "Please stand-by     ");

    syslog( LOG_DEBUG, "LCD opening statement" );
    sleep(5);

    while(TRUE)
    {
        //syslog( LOG_DEBUG, "Collecting time info" );

        /* get current time and format to string */
        time( &rawtime );
        current = localtime( &rawtime );
        ctm = asctime(current);
        //syslog( LOG_DEBUG, "Current time is %s", asctime(current) );
        for( ii=0, iii=0; iii<20; iii++ )
        {
            if( ii==2 )
            {
                ii += 1;
            }
            else if( ii==16 )
            {
                ii += 3;
            }
            ln1[iii] = ctm[ii];
            ii++;
        }

        /* check openvpn log for status info */
        //syslog( LOG_DEBUG, "Calling read log" );
        call_read_log_f( util_struct );
        if( util_struct->log_found == 1 )
        {
            for( ii=4; ii<20; ii++ )
            {
                ln3[ii] = util_struct->log_ip_addr[ii-4];
            }
        }
        /* if not found, write no connections */
        else
        {
            for( ii=0; ii<20; ii++ )
            {
                ln3[ii] = no_addr[ii];
            }
        }

        /* write info to different lines */
        write_lcd_line_f( fd, 1, ln1 );
        write_lcd_line_f( fd, 2, ln2 );
        write_lcd_line_f( fd, 3, ln3 );
        write_lcd_line_f( fd, 4, &ln4[ptr_cnt] );

        /* pointer to update project member names */
        ptr_cnt = ((ptr_cnt+20)%60);

        sleep(5);

        /* check for signal exit */
        if( util_struct->flag_exit == 1 )
        {
            break;
        }
    }

    /* write good-bye message */
    write_lcd_line_f( fd, 1, "Thanks for stopping ");
    write_lcd_line_f( fd, 2, "by and hope to see  ");
    write_lcd_line_f( fd, 3, "you again sometime. ");
    write_lcd_line_f( fd, 4, "Peace, I'm out!!!!! ");

    return 0;
}

/******************************************************************************/
/* Function to combine lseek and write into single call                       */
/******************************************************************************/
int write_lcd_line_f ( int fd, int line_num, char str[20] )
{
    /* check for valid handle */
    if( fd <= 0 )
    {
        return -1;
    }

    /* check for valid line */
    if( line_num < 1 || line_num > 4 )
    {
        return -1;
    }

    /* seek to correct line */
    if( lseek( fd, (line_num-1)*20, SEEK_SET ) == -1 )
    {
        return -1;
    }

    /* write string to lcd module */
    if( write( fd, str, 20 ) == -1 )
    {
        return -1;
    }

    return 0;
}
/******************************************************************************/
/* Function used to find the current device's IP address                      */
/*                                                                            */
/* Code mostly based on example that was found at the following link.  That   */
/* code generically printed out all of the addresses found through getifaddrs */
/* I added code to find specifically eth0 and/or wlan0 and format to string   */
/*                                                                            */
/* https://stackoverflow.com/questions/1570511/c-code-to-get-the-ip-address   */
/******************************************************************************/
int collect_ip_addr_f( char ip_addr[17] )
{
    struct ifaddrs *ifaddr, *ifa;
    int family, s;
    char host[NI_MAXHOST];

    int found = 0;

    syslog( LOG_DEBUG, "collect_ip_addr_f" );

    /* get all address info */
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        syslog( LOG_ERR, "ERROR - getifaddrs(%d)", errno );
        return -1;
    }

    syslog( LOG_DEBUG, "search ll" );

    /* search linked-list for specific addresses */
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        family = ifa->ifa_addr->sa_family;

        /* check family type */
        if (family == AF_INET) {
            s = getnameinfo
                (
                    ifa->ifa_addr, 
                    sizeof(struct sockaddr_in),
                    host, 
                    NI_MAXHOST, 
                    NULL, 
                    0, 
                    NI_NUMERICHOST
                );
            if (s != 0) {
                //printf("getnameinfo() failed: %s\n", gai_strerror(s));
                syslog( LOG_ERR, "getnameinfo() failed: %s", gai_strerror(s) );
                return -1;
            }
            syslog( LOG_DEBUG, "<Interface>: %s <Address> %s", ifa->ifa_name, host );
            //printf("<Interface>: %s \t <Address> %s\n", ifa->ifa_name, host);

            /* if wired address */
            if( strcmp(ifa->ifa_name, "eth0") == 0 )
            {
                syslog( LOG_DEBUG, "found eth0" );
                strcpy( ip_addr, host );
                found = 1;
            }
            /* if wifi address */
            else if( strcmp(ifa->ifa_name, "wlan0") == 0 )
            {
                syslog( LOG_DEBUG, "found wlan0" );
                strcpy( ip_addr, host );
                found = 1;
            }
        }
    }

    return found;
}
