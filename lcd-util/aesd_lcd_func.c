
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

    //int ii = 0;

    int rc;

    int cnt_i;
    int pid_i;
    int local_exit = 0;

    char buf[ TEST_BUFFER_SIZE ];

    char * fifo_p = "/tmp/aesd_lcd_fifo";

    struct pollfd poll_s;

    mkfifo( fifo_p, 0666 );

    util_struct->fd = open( fifo_p, O_RDONLY | O_NONBLOCK );

    poll_s.fd     = util_struct->fd;
    poll_s.events = POLLIN;

    while( TRUE )
    {
        rc = poll( &poll_s, 1, RD_VAL );

        if( (rc == 1) && ((poll_s.revents & 0x0001) == POLLIN) && (poll_s.revents < 32))
        {
            rd_sz = read( util_struct->fd, buf, 36/*sizeof(buf)*/ );

            if( (rd_sz < TEST_BUFFER_SIZE) )
            {
                buf[rd_sz] = '\0';
            }

            cnt_i = value_unpack( &buf[20] );
            pid_i = value_unpack( &buf[31] );

            syslog( LOG_DEBUG, "Read rec %s; (%d bytes, %d, %d)", 
                buf, 
                rd_sz, 
                cnt_i,
                pid_i
            );

            if( util_struct->a != NULL )
            {
                while( (util_struct->a[2] == 1) && (util_struct->a[3] == 0) && (util_struct->flag_exit == 0) )
                {
                    sleep(1);
                }

                util_struct->a[0] = cnt_i;
                util_struct->a[1] = pid_i;
                util_struct->a[2] = 1;

                if( util_struct->a[3] == 1 )
                {
                    local_exit = 1;
                }
            }
        }
        else if( rc == 0 )
        {
            syslog( LOG_DEBUG, "Read timeout" );
        }
        else
        {
            // syslog( LOG_DEBUG, "Read error with rc %d w/ revents %d, events %d, (%d)", rc, poll_s.revents, poll_s.events, errno );
            sleep( ONE );
        }

        if( util_struct->a != NULL )
        {
            if( util_struct->a[3] == 1 )
            {
                local_exit = 1;
            }
        }

        if( (util_struct->flag_exit == 1) || (local_exit == 1) )
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
        util_struct->read_f  = setup_read_f;
        util_struct->write_f = call_write_test_f;
    }
    else
    {
        util_struct->read_f  = test_func_1;//call_default_f;
        util_struct->write_f = call_default_f;
    }

    util_struct->daemon_f = call_daemon_f;

    util_struct->line1 = ASED_16x2_LINE1;
    util_struct->line2 = ASED_16x2_LINE2;

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

/**********************************************/
/* re-purposed functions from the lcd example */
/* in wiringPi                                */
/**********************************************/
void aesd_lcd_init( int input_fd )
{
    // Initialise display
    aesd_lcd_byte( input_fd, 0x33, AESD_LCD_CMD ); // Initialise
    aesd_lcd_byte( input_fd, 0x32, AESD_LCD_CMD ); // Initialise
    aesd_lcd_byte( input_fd, 0x06, AESD_LCD_CMD ); // Cursor move direction
    aesd_lcd_byte( input_fd, 0x0C, AESD_LCD_CMD ); // 0x0F On, Blink Off
    aesd_lcd_byte( input_fd, 0x28, AESD_LCD_CMD ); // Data length, number of lines, font size
    aesd_lcd_byte( input_fd, 0x01, AESD_LCD_CMD ); // Clear display
    delayMicroseconds(500);
}

void aesd_lcd_byte( int input_fd, int bits, int mode )
{
    //Send byte to data pins
    // bits = the data
    // mode = 1 for data, 0 for command
    int bits_high;
    int bits_low;
    // uses the two half byte writes to LCD
    bits_high = mode | (bits & 0xF0) | AESD_LCD_BACKLT;
    bits_low  = mode | ((bits << 4) & 0xF0) | AESD_LCD_BACKLT;

    // High bits
    wiringPiI2CReadReg8(input_fd, bits_high);
    aesd_lcd_toggle(input_fd, bits_high);

    // Low bits
    wiringPiI2CReadReg8(input_fd, bits_low);
    aesd_lcd_toggle(input_fd, bits_low);
}

void aesd_lcd_clear( int input_fd )
{
    aesd_lcd_byte( input_fd, 0x01, AESD_LCD_CMD );
    aesd_lcd_byte( input_fd, 0x02, AESD_LCD_CMD );
}

void aesd_lcd_loc( int input_fd, int line )
{
    aesd_lcd_byte( input_fd, line, AESD_LCD_CMD );
}

void aesd_lcd_type_ln( int input_fd, const char *s )
{
    while( *s )
    {
        aesd_lcd_byte( input_fd, *(s++), AESD_LCD_CHR);
    }
}

void aesd_lcd_toggle( int input_fd, int bits )
{
    // Toggle enable pin on LCD display
    delayMicroseconds(500);
    wiringPiI2CReadReg8(input_fd, (bits | AESD_LCD_ENABLE));
    delayMicroseconds(500);
    wiringPiI2CReadReg8(input_fd, (bits & ~AESD_LCD_ENABLE));
    delayMicroseconds(500);
}