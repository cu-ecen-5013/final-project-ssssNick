
#include "aesd_lcd_util.h"

/******************************************************************************/
/* Main program call                                                          */
/*                                                                            */
/* Highly simplified program that will setup a utility structure to handle    */
/* passing of information and directing which calls are used for given        */
/* situation.  Requires command line arguments to run.  Needs to be called    */
/* for at least -r for read mode or -w for write mode.  Can be called in test */
/* mode by including -t and any can be created into daemon processes using -d */
/* If no argument is given, it will fall through and exit with an error.      */
/*                                                                            */
/* Example code was either referenced or used from the following references:  */
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

int main( int argc, char **argv )
{
    /* structure used for everything */
    struct aesd_struct util_struct;

    /* setup default values for struct */
    /* parse command line parameters into struct */
    /* set function calls based on command line args */
    if( struct_setup_f( argc, argv, &util_struct ) < 0 )
    {
        goto main_exit_w_err;
    }

    /* setup as daemon process */
    if( util_struct.flag_d == 1 )
    {
        if( util_struct.daemon_f( &util_struct ) < 0 )
        {
            goto main_exit_w_err;
        }
        if( util_struct.pid_rtn > 0 )
        {
            goto main_exit;
        }
    }

    /* run read in read mode */
    if( util_struct.flag_r == 1 )
    {
        if( util_struct.read_f( &util_struct ) < 0 )
        {
            goto main_exit_w_err;
        }

        goto main_exit;
    }
    /* run in write mode */
    else if( util_struct.flag_w == 1 )
    {
        if( util_struct.write_f( &util_struct ) < 0 )
        {
            goto main_exit_w_err;
        }

        goto main_exit;
    }

// fall through and exit with error if neither rd or wr selected
main_exit_w_err:
    clean_up_f( &util_struct );

    return -1;

main_exit:
    clean_up_f( &util_struct );

    return 0;
}
