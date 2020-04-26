
#include "aesd_lcd_util.h"

/******************************************************************************/
/**/
/******************************************************************************/
int call_read_log_f ( struct aesd_struct *util_struct )
{
    FILE *log_fd;

    int ii = 0;

    int log_found = 0;

    char buffer[BUFFER_SIZE];
    char ipaddr[16] = "                ";
    char emptys[16] = "                ";

    char cmp_str[11] = "vpn.server,";

    syslog( LOG_DEBUG, "In call read log" );

    log_fd = fopen( "/var/log/openvpn-status.log", "r" );

    if( log_fd <= 0 )
    {
        syslog( LOG_ERR, "ERROR - OpenVPN status log cannot be opened (%d)", errno );
        util_struct->log_found = 0;
        return -1;
    }

    syslog( LOG_DEBUG, "OpenVPN log file opened @ %p", log_fd );

    while( fgets( buffer, sizeof(buffer), log_fd ) )
    {
        syslog( LOG_DEBUG, "OpenVPN log: %s", buffer );

        for( ii=0; ii<11; ii++ )
        {
            if( buffer[ii] != cmp_str[ii] )
            {   
                syslog( LOG_DEBUG, "OpenVPN log no match (%d)", ii );
                goto no_match;
            }
        }
        for( ii=11; ii<27; ii++ )
        {
            if( buffer[ii] == ':' )
            {
                goto match;
            }

            ipaddr[ii-11] = buffer[ii];
        }
match:
        log_found = 1;
        syslog( LOG_DEBUG, "OpenVPN log set value with %s (%d)", ipaddr, ii );
        break;
no_match:
        syslog( LOG_DEBUG, "continue" );
    }

    fclose( log_fd );

    if( log_found == 1 )
    {
        strcpy( util_struct->log_ip_addr, ipaddr );
        util_struct->log_found = 1;
        syslog( LOG_DEBUG, "Log found set");
    }
    else
    {
        strcpy( util_struct->log_ip_addr, emptys );
        util_struct->log_found = 0;
        syslog( LOG_DEBUG, "Log found cleared");
    }

    return 0;
}