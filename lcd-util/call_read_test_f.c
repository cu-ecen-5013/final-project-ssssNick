
#include "aesd_lcd_util.h"

/******************************************************************************/
/**/
/******************************************************************************/
int call_read_test_f( struct aesd_struct *util_struct )
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

        //call_read_test_f( util_struct );
        //call_read_pipe_f( util_struct );
        util_struct->read_pipe_f( util_struct );

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
        //call_write_lcd_f( util_struct );
        util_struct->write_lcd_f( util_struct );
        
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

// int call_read_test_f( struct aesd_struct *util_struct )
// {
//     syslog( LOG_DEBUG, "Read test function called" );

//     int rd_sz = 0;

//     //int ii = 0;

//     int rc;

//     int cnt_i;
//     int pid_i;
//     int local_exit = 0;

//     char buf[ TEST_BUFFER_SIZE ];

//     char * fifo_p = "/tmp/aesd_lcd_fifo";

//     struct pollfd poll_s;

//     mkfifo( fifo_p, 0666 );

//     util_struct->fd = open( fifo_p, O_RDONLY | O_NONBLOCK );

//     poll_s.fd     = util_struct->fd;
//     poll_s.events = POLLIN;

//     while( TRUE )
//     {
//         rc = poll( &poll_s, 1, RD_VAL );

//         if( (rc == 1) && ((poll_s.revents & 0x0001) == POLLIN) && (poll_s.revents < 32))
//         {
//             rd_sz = read( util_struct->fd, buf, 36/*sizeof(buf)*/ );

//             if( (rd_sz < TEST_BUFFER_SIZE) )
//             {
//                 buf[rd_sz] = '\0';
//             }

//             cnt_i = value_unpack( &buf[20] );
//             pid_i = value_unpack( &buf[31] );

//             syslog( LOG_DEBUG, "Read rec %s; (%d bytes, %d, %d)", 
//                 buf, 
//                 rd_sz, 
//                 cnt_i,
//                 pid_i
//             );

//             if( util_struct->a != NULL )
//             {
//                 while( (util_struct->a[2] == 1) && (util_struct->a[3] == 0) && (util_struct->flag_exit == 0) )
//                 {
//                     sleep(1);
//                 }

//                 util_struct->a[0] = cnt_i;
//                 util_struct->a[1] = pid_i;
//                 util_struct->a[2] = 1;

//                 if( util_struct->a[3] == 1 )
//                 {
//                     local_exit = 1;
//                 }
//             }
//         }
//         else if( rc == 0 )
//         {
//             syslog( LOG_DEBUG, "Read timeout" );
//         }
//         else
//         {
//             // syslog( LOG_DEBUG, "Read error with rc %d w/ revents %d, events %d, (%d)", rc, poll_s.revents, poll_s.events, errno );
//             sleep( ONE );
//         }

//         if( util_struct->a != NULL )
//         {
//             if( util_struct->a[3] == 1 )
//             {
//                 local_exit = 1;
//             }
//         }

//         if( (util_struct->flag_exit == 1) || (local_exit == 1) )
//         {
//             close( util_struct->fd );
//             break;
//         }
//     }

//     return 0;
// }