
#include "aesd_lcd_util.h"

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

    util_struct->fd     = 0;
    util_struct->lcd_fd = 0;

    util_struct->pack_size = 0;

    flag_to_exit = &util_struct->flag_exit;

    util_struct->a = NULL;
    util_struct->b = NULL;

    util_struct->head = NULL;

    signal( SIGINT,  common_sig_handle_f );
    signal( SIGTERM, common_sig_handle_f );

    openlog( "aesd_lcd_util_logs", LOG_PID, LOG_USER );

    while( (param = getopt( argc, argv, "dlrtws:" )) != -1 )
    {
        switch( param )
        {
            case 'd':
                util_struct->flag_d = 1;
                break;
            case 'l':
                util_struct->flag_l = 1;
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
        util_struct->read_f       = call_read_test_f;//setup_read_f;
        util_struct->read_pipe_f  = call_read_pipe_f;
        util_struct->write_f      = call_write_test_f;
        util_struct->write_lcd_f  = call_write_i2c_f;
        util_struct->write_pipe_f = call_write_pipe_f;
    }
    else
    {
        util_struct->read_f       = test_func_1;
        util_struct->read_pipe_f  = call_default_f;
        util_struct->write_f      = call_write_lcd_f;
        util_struct->write_lcd_f  = call_default_f;
        util_struct->write_pipe_f = call_default_f;
    }

    util_struct->daemon_f = call_daemon_f;

    if( util_struct->flag_l == 1 )
    {        
        util_struct->line1 = ASED_16x2_LINE1;
        util_struct->line2 = ASED_16x2_LINE2;
        util_struct->line3 = 0x00;
        util_struct->line4 = 0x00;
    }
    else
    {
        // util_struct->line1 = ASED_20x4_LINE1;
        // util_struct->line2 = ASED_20x4_LINE2;
        // util_struct->line3 = ASED_20x4_LINE3;
        // util_struct->line4 = ASED_20x4_LINE4;

        util_struct->line1 = ASED_16x2_LINE1;
        util_struct->line2 = ASED_16x2_LINE2;
        util_struct->line3 = 0x00;
        util_struct->line4 = 0x00;
    }

    if( util_struct->flag_t == 0 )
    {
        util_struct->lcd_fd = open( "/dev/lcd", O_RDWR );

        if( util_struct->lcd_fd <= 0 )
        {
            syslog( LOG_ERR, "ERROR - Unable to open LCD device (%d)", errno );
            return -1;
        }
    }

    return 0;
}