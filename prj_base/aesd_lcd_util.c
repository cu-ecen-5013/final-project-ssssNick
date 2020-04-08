
#include "aesd_lcd_util.h"

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