
#include "aesd_lcd_util.h"

/******************************************************************************/
/**/
/******************************************************************************/
int setup_read_f( struct aesd_struct *util_struct )
{
    int pid;

    util_struct->shmid = shmget(IPC_PRIVATE, 4*sizeof(int), 0777|IPC_CREAT);;
    syslog( LOG_DEBUG, "Shared memory obtained" ); 

    pid = fork();
    if( pid < 0 )
    {
        syslog( LOG_ERR, "ERROR - Error when fork attempted(%d)", errno );
        return -1;
    }

    if( pid > 0 )
    {
        util_struct->a = (int *) shmat(util_struct->shmid, 0, 0);

        util_struct->a[0] = 0;
        util_struct->a[1] = 0;
        util_struct->a[2] = 0;
        util_struct->a[3] = 0;

        call_read_test_f( util_struct );
        
        if( util_struct->a[3] == 1 )
        {
            shmdt( util_struct->a );
            shmctl( util_struct->shmid, IPC_RMID, 0 ); 
            syslog( LOG_DEBUG, "Shared memory detached and destroyed" );    
        }
        else
        {
            util_struct->a[3] = 1;
            shmdt( util_struct->a );
            syslog( LOG_DEBUG, "Shared memory detached" );  
        }
    }
    else
    {
        util_struct->a = (int *) shmat(util_struct->shmid, 0, 0);

        //rd_shared_mem_f( util_struct );
        call_write_lcd_f( util_struct );

        //util_struct->write_lcd_f( util_struct );
    
        if( util_struct->a[3] == 1 )
        {
            shmdt( util_struct->a );
            shmctl( util_struct->shmid, IPC_RMID, 0 );
            syslog( LOG_DEBUG, "Shared memory detached and destroyed" );  
        }
        else
        {
            util_struct->a[3] = 1;
            shmdt( util_struct->a );
            syslog( LOG_DEBUG, "Shared memory detached" );  
        }
    }

    return 0;
}