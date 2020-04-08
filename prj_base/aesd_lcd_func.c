
#include "aesd_lcd_util.h"

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

    int ii;

    int rc;

    char buf[ TEST_BUFFER_SIZE ];

    char * fifo_p = "/tmp/aesd_lcd_fifo";

    struct pollfd poll_s;

    mkfifo( fifo_p, 0666 );

    util_struct->fd = open( fifo_p, O_RDONLY | O_NONBLOCK );

    poll_s.fd     = util_struct->fd;
    poll_s.events = POLLIN;

    while( TRUE )
    {

        rc = poll( &poll_s, 1, 5000 );

        if( rc == 1 && poll_s.revents == POLLIN )
        {
            rd_sz = read( util_struct->fd, buf, sizeof(buf) );

            if( (rd_sz == 25) && (buf[24] == '\0') )
            {
                ii = value_unpack( &buf[20] );
                
                syslog( LOG_DEBUG, "Read rec %s; %d with %d bytes", buf, ii, rd_sz );
            }
            else
            {
                ii = 0;
                
                buf[rd_sz] = '\0';

                syslog( LOG_DEBUG, "Read rec %s; %d bytes", buf, rd_sz );
            }
        }

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

    int ii  = 0;
    int val = 0;

    // int wr_sz = 0;

    char buf[ TEST_BUFFER_SIZE ] = "my current value is ";
  
    // FIFO file path 
    char * fifo_p = "/tmp/aesd_lcd_fifo"; 

    mkfifo( fifo_p, 0666 );

    while( TRUE )
    {  
        val = 5;
        while( val > 0 )
        {
            val = sleep( val );
        }

        util_struct->fd = open( fifo_p, O_WRONLY | O_NONBLOCK );
        if( util_struct->fd >= 0 )
        {
            ii = (ii+5)%10000;

            value_pack( ii, &buf[20] );

            write(  util_struct->fd, buf, sizeof(buf) );

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
    closelog();
}
/******************************************************************************/
/**/
/******************************************************************************/
void common_sig_handle_f( int sig )
{
    syslog( LOG_DEBUG, "Sig %d caught", sig );

    *flag_to_exit = 1;
}
/******************************************************************************/
/**/
/******************************************************************************/
void value_pack( int input_val, char *buf )
{
    buf[3] = 48 + ((input_val/1)%10);
    buf[2] = 48 + ((input_val/10)%10);
    buf[1] = 48 + ((input_val/100)%10);
    buf[0] = 48 + ((input_val/1000)%10);
}
/******************************************************************************/
/**/
/******************************************************************************/
int value_unpack( char *buf )
{
    int val = 0;

    val += 1*(buf[3]-48);
    val += 10*(buf[2]-48);
    val += 100*(buf[1]-48);
    val += 1000*(buf[0]-48);

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
    util_struct->flag_t = 0;
    util_struct->flag_w = 0;

    util_struct->flag_exit = 0;

    util_struct->pid_rtn = 0;

    util_struct->fd = 0;

    flag_to_exit = &util_struct->flag_exit;

    signal( SIGINT,  common_sig_handle_f );
    signal( SIGTERM, common_sig_handle_f );

    openlog( "aesd_lcd_util_logs", LOG_PID, LOG_USER );

    while( (param = getopt( argc, argv, "drtw" )) != -1 )
    {
        switch( param )
        {
            case 'd':
                util_struct->flag_d = 1;
                break;
            case 'r':
                util_struct->flag_r = 1;
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
        util_struct->write_f = call_default_f;
    }

    util_struct->daemon_f = call_daemon_f;

    return 0;
}