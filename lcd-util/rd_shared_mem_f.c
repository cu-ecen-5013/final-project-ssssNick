
#include "aesd_lcd_util.h"

/******************************************************************************/
/**/
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