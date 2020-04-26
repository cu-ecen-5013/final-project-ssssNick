
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
/* Function used for testing out the system using the named pipe and i2c      */
/*                                                                            */
/* Will setup a shared memory between 2 forked processes.                     */
/******************************************************************************/
int call_read_test_f( struct aesd_struct *util_struct )
{
    int pid;

    /* handle for shared memory */
    util_struct->shmid = shmget(IPC_PRIVATE, 4*sizeof(int), 0777|IPC_CREAT);;
    syslog( LOG_DEBUG, "Shared memory obtained" ); 

    /* fork into 2 processes */
    pid = fork();
    if( pid < 0 )
    {
        syslog( LOG_ERR, "ERROR - Error when fork attempted(%d)", errno );
        return -1;
    }

    /* 1st forked process */
    if( pid > 0 )
    {
        /* setup shared memory and clear */
        util_struct->a = (int *) shmat(util_struct->shmid, 0, 0);

        util_struct->a[0] = 0;
        util_struct->a[1] = 0;
        util_struct->a[2] = 0;
        util_struct->a[3] = 0;

        /* call the read_pipe function pointer */
        util_struct->read_pipe_f( util_struct );

        /* if shared memory already detached by other process */
        /* then will need to destroy after detaching */
        if( util_struct->a[3] == 1 )
        {
            shmdt( util_struct->a );
            shmctl( util_struct->shmid, IPC_RMID, 0 ); 
            syslog( LOG_DEBUG, "Shared memory detached and destroyed" );    
        }
        /* if not already detached, will need to detach and flag */
        /* other process to destroy after detaching */
        else
        {
            util_struct->a[3] = 1;
            shmdt( util_struct->a );
            syslog( LOG_DEBUG, "Shared memory detached" );  
        }
    }
    /* 2nd forked proecces */
    else
    {
        /* setup shared memory and clear */
        util_struct->a = (int *) shmat(util_struct->shmid, 0, 0);

        /* call the write lcd function pointer */
        util_struct->write_lcd_f( util_struct );

        /* if shared memory already detached by other process */
        /* then will need to destroy after detaching */
        if( util_struct->a[3] == 1 )
        {
            shmdt( util_struct->a );
            shmctl( util_struct->shmid, IPC_RMID, 0 );
            syslog( LOG_DEBUG, "Shared memory detached and destroyed" );  
        }
        /* if not already detached, will need to detach and flag */
        /* other process to destroy after detaching */
        else
        {
            util_struct->a[3] = 1;
            shmdt( util_struct->a );
            syslog( LOG_DEBUG, "Shared memory detached" );  
        }
    }

    return 0;
}