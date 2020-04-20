
#include "aesd_lcd_util.h"

/******************************************************************************/
/**/
/******************************************************************************/
int call_write_test_f( struct aesd_struct *util_struct )
{
    syslog( LOG_DEBUG, "Write function called" );

    int val = 0;

    int ii;

    int pid = getpid();

    if( util_struct->flag_s == 1 )
    {
        ii = util_struct->seed_val;
    }
    else
    {
        ii = 0;
    }

    char buf[ TEST_BUFFER_SIZE ] = "my current value is _____ from _____";
  
    // FIFO file path 
    char * fifo_p = "/tmp/aesd_lcd_fifo"; 

    mkfifo( fifo_p, 0666 );

    while( TRUE )
    {  
        val = WR_VAL;
        while( val > 0 )
        {
            val = sleep( val );

            if( util_struct->flag_exit == 1 )
            {
                return 0;
            }
        }

        util_struct->fd = open( fifo_p, O_WRONLY | O_NONBLOCK );
        if( util_struct->fd >= 0 )
        {
            ii = (ii+5)%100000;

            value_pack( ii, &buf[20] );
            value_pack( pid, &buf[31] );

            write(  util_struct->fd, buf, 36 /*sizeof(buf)*/ );

            syslog( LOG_DEBUG, "Wrote %s", buf);

            close( util_struct->fd );

            util_struct->fd = 0;
        }
        else
        {
            syslog( LOG_DEBUG, "Not FIFO connect (%d); Sleep and retry", util_struct->fd );
        }

        if( util_struct->flag_exit == 1 )
        {
            break;
        }
    }

    return 0;
}