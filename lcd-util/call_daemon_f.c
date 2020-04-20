
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