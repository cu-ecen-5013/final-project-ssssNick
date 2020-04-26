
#include "aesd_lcd_util.h"

int write_lcd_line_f ( int fd, int line_num, char str[20] );
int collect_ip_addr_f( char ip_addr[17] );

int call_write_lcd_f ( struct aesd_struct *util_struct )
{
    int ii,iii;

    int ptr_cnt = 0;
    int fd = 0;

    /* for finding system readable time */
    time_t rawtime;
    struct tm *current;
    char *ctm;

    /* template for each lcd line */
    char ip_addr [17] = "                 ";
    char ln1 [20] = "                    ";
    char ln2 [20] = "IP                  ";
    char ln3 [20] = "No VPN connections  ";
    char ln4 [60] = ">  Ayden Blotnick  <>  Nick Brubaker   <> Martin Lennartz  <";
    //char ety [20] = "                    ";
    //               012345678901234567890123456789012345678901234567890123456789
    //               0         10        20        30        40        50        
    syslog( LOG_DEBUG, "call_write_lcd_f" );

    //time( &rawtime );

    collect_ip_addr_f( ip_addr );

    for( ii=0; ii<17; ii++ )
    {
        if( ip_addr[ii] == '\0' )
        {
            break;
        }
        ln2[ii+3] = ip_addr[ii];
    }

    sleep(5);

    // fd = open( "/dev/lcd", O_RDWR );
    fd = util_struct->lcd_fd;
    
    if( fd < 0 )
    {
        syslog( LOG_ERR, "ERROR - Failed to open LCD file descriptor with returned fd of %d (%d)", fd , errno );
    }

    syslog( LOG_DEBUG, "LCD opened" );

    write_lcd_line_f( fd, 1, "Setting everything  ");
    write_lcd_line_f( fd, 2, "up so we can see wha");
    write_lcd_line_f( fd, 3, "t is going to happen");
    write_lcd_line_f( fd, 4, "Please stand-by     ");
    //                        01234567890123456789
    syslog( LOG_DEBUG, "LCD opening statement" );
    sleep(5);

    while(TRUE)
    {
        time( &rawtime );
        current = localtime( &rawtime );
        ctm = asctime(current);
        //syslog( LOG_DEBUG, "Current time is %s", asctime(current) );
        for( ii=0, iii=0; iii<20; iii++ )
        {
            if( ii==2 )
            {
                ii += 1;
            }
            else if( ii==16 )
            {
                ii += 3;
            }
            ln1[iii] = ctm[ii];
            ii++;
        }

        write_lcd_line_f( fd, 1, ln1 );
        write_lcd_line_f( fd, 2, ln2 );
        write_lcd_line_f( fd, 3, ln3 );
        write_lcd_line_f( fd, 4, &ln4[ptr_cnt] );

        ptr_cnt = ((ptr_cnt+20)%60);

        //sleep(1);
        sleep(5);
        if( util_struct->flag_exit == 1 )
        {
            break;
        }
    }

    write_lcd_line_f( fd, 1, "Thanks for stopping ");
    write_lcd_line_f( fd, 2, "by and hope to see  ");
    write_lcd_line_f( fd, 3, "you again sometime. ");
    write_lcd_line_f( fd, 4, "Peace, I'm out!!!!! ");

    // close( fd );
    // util_struct->i2c_fd = 0;

    return 0;
}

int write_lcd_line_f ( int fd, int line_num, char str[20] )
{
    if( fd <= 0 )
    {
        return -1;
    }

    if( line_num < 1 || line_num > 4 )
    {
        return -1;
    }

    if( lseek( fd, (line_num-1)*20, SEEK_SET ) == -1 )
    {
        return -1;
    }
    if( write( fd, str, 20 ) == -1 )
    {
        return -1;
    }

    return 0;
}

/*https://stackoverflow.com/questions/1570511/c-code-to-get-the-ip-address*/
int collect_ip_addr_f( char ip_addr[17] )
{
    struct ifaddrs *ifaddr, *ifa;
    int family, s;
    char host[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        syslog( LOG_ERR, "ERROR - getifaddrs(%d)", errno );
        //exit(EXIT_FAILURE);
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        family = ifa->ifa_addr->sa_family;

        if (family == AF_INET) {
            s = getnameinfo
                (
                    ifa->ifa_addr, 
                    sizeof(struct sockaddr_in),
                    host, 
                    NI_MAXHOST, 
                    NULL, 
                    0, 
                    NI_NUMERICHOST
                );
            if (s != 0) {
                //printf("getnameinfo() failed: %s\n", gai_strerror(s));
                syslog( LOG_ERR, "getnameinfo() failed: %s", gai_strerror(s) );
                exit(EXIT_FAILURE);
            }
            syslog( LOG_DEBUG, "<Interface>: %s <Address> %s", ifa->ifa_name, host );
            //printf("<Interface>: %s \t <Address> %s\n", ifa->ifa_name, host);
            if( strcmp(ifa->ifa_name, "eth0") == 0 )
            {
                syslog( LOG_DEBUG, "found eth0" );
                strcpy( ip_addr, host );
            }
        }
    }

    return 0;
}