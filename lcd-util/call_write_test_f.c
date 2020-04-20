
#include "aesd_lcd_util.h"

/******************************************************************************/
/**/
/******************************************************************************/
int call_write_test_f( struct aesd_struct *util_struct )
{
    return util_struct->write_pipe_f( util_struct );
}