
#include "aesd_lcd_util.h"

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
/* Function used to parse the command line arguments and setup the structure  */
/* used by all of the functions                                               */
/******************************************************************************/
int struct_setup_f( int argc, char **argv, struct aesd_struct *util_struct )
{
    int param = 0;

    /* setting flags and pointers to default values */
    util_struct->flag_d = 0;
    util_struct->flag_r = 0;
    util_struct->flag_s = 0;
    util_struct->flag_t = 0;
    util_struct->flag_w = 0;

    util_struct->flag_exit = 0;

    util_struct->pid_rtn = 0;

    util_struct->fd     = 0;
    util_struct->lcd_fd = 0;

    util_struct->pack_size = 0;

    /* connecting signal flag to structure */
    /* will allow signal handler call to be */
    /* detected structure the structure */
    flag_to_exit = &util_struct->flag_exit;

    /* shared memory pointers */
    util_struct->a = NULL;
    util_struct->b = NULL;

    /* linked-list pointer */
    util_struct->head = NULL;

    /* signal handlers */
    /* not part of structure but obvious place */
    /* to setup */
    signal( SIGINT,  common_sig_handle_f );
    signal( SIGTERM, common_sig_handle_f );

    /* open syslogs */
    openlog( "aesd_lcd_util_logs", LOG_PID, LOG_USER );

    /* parse command line arguments */
    while( (param = getopt( argc, argv, "dlrtws:" )) != -1 )
    {
        switch( param )
        {
            case 'd':
                util_struct->flag_d = 1;
                break;
            case 'l':
                util_struct->flag_l = 1;
                break;
            case 'r':
                util_struct->flag_r = 1;
                break;
            case 's':
                util_struct->flag_s   = 1;
                util_struct->seed_val = atoi( optarg );
                break;
            case 't':
                util_struct->flag_t = 1;
                break;
            case 'w':
                util_struct->flag_w = 1;
                break;
            case '?':
                // unknown para
                return -1;
            default:
                break;
        }
    }

    /* setup function pointers */
    if( util_struct->flag_t == 1 )
    {
        /* for test mode */
        util_struct->read_f       = call_read_test_f;
        util_struct->read_pipe_f  = call_read_pipe_f;
        util_struct->write_f      = call_write_test_f;
        util_struct->write_lcd_f  = call_write_i2c_f;
        util_struct->write_pipe_f = call_write_pipe_f;
    }
    else
    {
        /* for all other modes */
        util_struct->read_f       = call_default_f;
        util_struct->read_pipe_f  = call_default_f;
        util_struct->write_f      = call_write_lcd_f;
        util_struct->write_lcd_f  = call_default_f;
        util_struct->write_pipe_f = call_default_f;
    }
    /* common */
    util_struct->daemon_f = call_daemon_f;

    /* i2c line addresses */
    util_struct->line1 = ASED_16x2_LINE1;
    util_struct->line2 = ASED_16x2_LINE2;

    /* LCD device file-descriptor */
    /* setup before using */
    if( util_struct->flag_t == 0 )
    {
        util_struct->lcd_fd = open( "/dev/lcd", O_RDWR );

        if( util_struct->lcd_fd <= 0 )
        {
            syslog( LOG_ERR, "ERROR - Unable to open LCD device (%d)", errno );
            return -1;
        }
    }

    return 0;
}