
#include <ctype.h>
#include <errno.h>
#include <fcntl.h> 
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <syslog.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define FALSE (0)
#define TRUE  (1)

#define ZERO  (0)
#define ONE   (1)
#define TWO   (2)
#define FIVE  (5)
#define K_1   (1000)

#define RD_VAL (TWO * K_1)
#define WR_VAL (FIVE * FIVE)

#define TEST_BUFFER_SIZE (85)

/* global exit flag; set by signal handler */
int *flag_to_exit;

/* structure to be passed around */
struct aesd_struct
{
    /* run flag */
    int flag_d;     /* daemon flag  */
    int flag_r;     /* read flag    */
    int flag_s;     /* seed flag    */
    int flag_t;     /* test flag    */
    int flag_w;     /* write flag   */

    /* exit flag set by sig handler */
    int flag_exit;

    int seed_val;

    /* pid returned from fork, if called */
    int pid_rtn;

    /* */
    int fd;

    int shmid;
    int status;

    int *a;
    int *b;

    int  pack_size;
    char pack_data[TEST_BUFFER_SIZE];

    /* function handlers */
    int (*daemon_f)( struct aesd_struct * );
    int (*read_f)  ( struct aesd_struct * );
    int (*write_f) ( struct aesd_struct * );
};

int call_daemon_f    ( struct aesd_struct * );
int call_default_f   ( struct aesd_struct * );
int func1            ( struct aesd_struct * );
int func2            ( struct aesd_struct * );
int call_read_lcd_f  ( struct aesd_struct * );
int call_read_test_f ( struct aesd_struct * );
int call_write_lcd_f ( struct aesd_struct * );
int call_write_test_f( struct aesd_struct * );
int call_write_2s_f  ( struct aesd_struct * );

/* close and clean-up anything needed before exiting */
void clean_up_f           ( struct aesd_struct *util_struct );

/* common signal handler */
void common_sig_handle_f  ( int sig );


/* set default values for flags */
/* connect global exit flag */
/* parse input parameters */
/* setup function pointers */
int struct_setup_f ( int , char **, struct aesd_struct * );


void value_pack   ( int input_val, char *buf );
int  value_unpack ( char *buf );

/******************************************************************************/
/**/
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

    if( util_struct.flag_r == 1 )
    {
        if( util_struct.read_f( &util_struct ) < 0 )
        {
            goto main_exit_w_err;
        }

        goto main_exit;
    }
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


/******************************************************************************/
/**/
/******************************************************************************/
int call_daemon_f( struct aesd_struct *util_struct )
{
    int pid;

    syslog( LOG_DEBUG, "Daemon function called" );

    pid = fork();
    if( pid < 0 )
    {
        syslog( LOG_ERR, "ERROR - Error when fork attempted(%d)", errno );
        return -1;
    }

    util_struct->pid_rtn = pid;

    if( pid > 0 )
    {
        syslog( LOG_DEBUG, "Returning since parent" );
        return 0;
    }

    umask(0);

    // Set session id to disconnect from terminal
    if( setsid() < 0 )
    {
        syslog( LOG_ERR, "ERROR - Error when attempting to set session id(%d)", errno );
        return -1;
    }

    // change working dir to root
    if( chdir("/") < 0 )
    {
        syslog( LOG_ERR, "ERROR - Error when attempting to change working dir to root(%d)", errno );
        return -1;
    }

    // Close out the standard file descriptors
    // doesn't match class notes where we should redirect to NULL
    if( close(STDIN_FILENO) == -1 )
    {
        syslog( LOG_ERR, "ERROR - Unable to close STDIN(%d)", errno );
        return -1;
    }
    if( close(STDOUT_FILENO) == -1 )
    {
        syslog( LOG_ERR, "ERROR - Unable to close STDOUT(%d)", errno );
        return -1;
    }
    if( close(STDERR_FILENO) == -1 )
    {
        syslog( LOG_ERR, "ERROR - Unable to close STDERR(%d)", errno );
        return -1;
    }

    return 0;

}
/******************************************************************************/
/**/
/******************************************************************************/
int call_default_f( struct aesd_struct *util_struct )
{
    syslog( LOG_DEBUG, "Default function called" );

    syslog( LOG_DEBUG, " - Not currently setup to run any additional functionality" );

    return 0;
}
/******************************************************************************/
/**/
/******************************************************************************/
int call_read_test_f( struct aesd_struct *util_struct )
{
    syslog( LOG_DEBUG, "Read test function called" );

    int rd_sz = 0;

    int ii = 0;

    int rc;

    char buf[ TEST_BUFFER_SIZE ];

    char * fifo_p = "/tmp/aesd_lcd_fifo";

    struct pollfd poll_s;

    mkfifo( fifo_p, 0666 );

    util_struct->fd = open( fifo_p, O_RDONLY | O_NONBLOCK );

    poll_s.fd     = util_struct->fd;
    poll_s.events = POLLIN;

    while( TRUE )
    {/*
        while( TRUE )
        {*/
            rc = poll( &poll_s, 1, RD_VAL );

            if( (rc == 1) && ((poll_s.revents & 0x0001) == POLLIN) && (poll_s.revents < 32))
            {
                rd_sz = read( util_struct->fd, buf, 36/*sizeof(buf)*/ );

                if( (rd_sz < TEST_BUFFER_SIZE) )
                {
                    buf[rd_sz] = '\0';
                }

                syslog( LOG_DEBUG, "Read rec %s; (%d bytes, %d, %d)", 
                    buf, 
                    rd_sz, 
                    value_unpack( &buf[20] ), 
                    value_unpack( &buf[31] ) 
                );
            }
            else if( rc == 0 )
            {
                syslog( LOG_DEBUG, "Read timeout %d", ii++ );
                /*break;*/
            }
            else
            {
                // syslog( LOG_DEBUG, "Read error with rc %d w/ revents %d, events %d, (%d)", rc, poll_s.revents, poll_s.events, errno );
                sleep( ONE );
            }
        /*}

        sleep(1);*/

        if( util_struct->flag_exit == 1 )
        {
            close( util_struct->fd );
            break;
        }
    }

    return 0;
}
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
/******************************************************************************/
/**/
/******************************************************************************/
void clean_up_f( struct aesd_struct *util_struct )
{
    syslog( LOG_DEBUG, "Cleaning up aesd util" );

    syslog( LOG_DEBUG, "Closing log" );    
    closelog();
}
/******************************************************************************/
/**/
/******************************************************************************/
void common_sig_handle_f( int sig )
{
    /*syslog( LOG_DEBUG, "Sig %d caught", sig );*/

    *flag_to_exit = 1;
}
/******************************************************************************/
/**/
/******************************************************************************/
void value_pack( int input_val, char *buf )
{
    /*syslog( LOG_DEBUG, "Packing Value %d", input_val );*/

    buf[4] = 48 + ((input_val/1)%10);
    buf[3] = 48 + ((input_val/10)%10);
    buf[2] = 48 + ((input_val/100)%10);
    buf[1] = 48 + ((input_val/1000)%10);
    buf[0] = 48 + ((input_val/10000)%10);
}
/******************************************************************************/
/**/
/******************************************************************************/
int value_unpack( char *buf )
{
    int val = 0;

    /*syslog( LOG_DEBUG, "Unpacking %c%c%c%c into Value", buf[0],buf[1],buf[2],buf[3] );*/

    val += 1*(buf[4]-48);
    val += 10*(buf[3]-48);
    val += 100*(buf[2]-48);
    val += 1000*(buf[1]-48);
    val += 10000*(buf[0]-48);

    return val;
}
/******************************************************************************/
/**/
/******************************************************************************/
int struct_setup_f( int argc, char **argv, struct aesd_struct *util_struct )
{
    int param = 0;

    util_struct->flag_d = 0;
    util_struct->flag_r = 0;
    util_struct->flag_s = 0;
    util_struct->flag_t = 0;
    util_struct->flag_w = 0;

    util_struct->flag_exit = 0;

    util_struct->pid_rtn = 0;

    util_struct->fd = 0;

    util_struct->pack_size = 0;

    flag_to_exit = &util_struct->flag_exit;

    util_struct->a = NULL;
    util_struct->b = NULL;

    signal( SIGINT,  common_sig_handle_f );
    signal( SIGTERM, common_sig_handle_f );

    openlog( "aesd_lcd_util_logs", LOG_PID, LOG_USER );

    while( (param = getopt( argc, argv, "drtws:" )) != -1 )
    {
        switch( param )
        {
            case 'd':
                util_struct->flag_d = 1;
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

    if( util_struct->flag_t == 1 )
    {
        util_struct->read_f  = call_read_test_f;
        util_struct->write_f = call_write_test_f;
    }
    else
    {
        util_struct->read_f  = call_default_f;
        util_struct->write_f = call_write_2s_f;//call_default_f;
    }

    util_struct->daemon_f = call_daemon_f;

    return 0;
}

/******************************************************************************/
/**/
/******************************************************************************/
int call_read_lcd_f( struct aesd_struct *util_struct )
{
    return 0;
}

/******************************************************************************/
/**/
/******************************************************************************/
int call_write_lcd_f( struct aesd_struct *util_struct )
{
    return 0;
}

int call_write_2s_f( struct aesd_struct *util_struct )
{
    int pid;
    int rtn = 0;

    util_struct->shmid = shmget(IPC_PRIVATE, 3*sizeof(int), 0777|IPC_CREAT);;

    pid = fork();
    if( pid < 0 )
    {
        syslog( LOG_ERR, "ERROR - Error when fork attempted(%d)", errno );
        return -1;
    }

    if( pid > 0 )
    {
        util_struct->b = (int *) shmat(util_struct->shmid, 0, 0);

        util_struct->b[0] = 0;
        util_struct->b[1] = 0;
        util_struct->b[2] = 0;

        func1( util_struct );

        rtn = util_struct->b[0];
        
        shmdt( util_struct->b );

        if(rtn == 1)
        {
            shmctl( util_struct->shmid, IPC_RMID, 0 );   
        }
    }
    else
    {
        util_struct->a = (int *) shmat(util_struct->shmid, 0, 0);

        func2( util_struct );

        rtn = util_struct->a[0];

        shmdt( util_struct->a );

        shmctl( util_struct->shmid, IPC_RMID, 0 );
    }

    return 0;
}

int func1( struct aesd_struct *util_struct )
{
    printf("Func 1 screen with packet @%p\n", util_struct );
    syslog( LOG_DEBUG, "Func 1 screen with packet @%p", util_struct );

    syslog( LOG_DEBUG, "1 Data packet size = %d (%p)", *util_struct->b, util_struct );
    sleep(1);

    *util_struct->b += 1;

    sleep(5);

    syslog( LOG_DEBUG, "1 Data packet size = %d (%p)", *util_struct->b, util_struct );
    *util_struct->b += 5;

    sleep(5);
    syslog( LOG_DEBUG, "1 Data packet size = %d (%p)", *util_struct->b, util_struct );

    *util_struct->b += 5;

    sleep(5);

    syslog( LOG_DEBUG, "1 Data packet size = %d (%p)", *util_struct->b, util_struct );

    return 0;
}

int func2( struct aesd_struct *util_struct )
{
    printf("Func 2 screen with packet @%p\n", util_struct );
    syslog( LOG_DEBUG, "Func 2 screen with packet @%p", util_struct );

    sleep(2);
    syslog( LOG_DEBUG, "2 Data packet size = %d (%p)", *util_struct->a, util_struct );
    sleep(5);
    syslog( LOG_DEBUG, "2 Data packet size = %d (%p)", *util_struct->a, util_struct );
    sleep(5);
    syslog( LOG_DEBUG, "2 Data packet size = %d (%p)", *util_struct->a, util_struct );
    *util_struct->a = 55;
    sleep(20);

    return 0;
}