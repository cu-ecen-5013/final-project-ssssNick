
#include "aesd_lcd_util.h"

int call_write_lcd_f ( struct aesd_struct *util_struct )
{
    int cnt_i;
    int pid_i;

    char ln1 [16] = "From PID _____";
    char ln2 [16] = "Rec val _____";

    syslog( LOG_DEBUG, "Func 2 screen with packet @%p", util_struct );

    if (wiringPiSetup () == -1) exit (1);

    util_struct->i2c_fd = wiringPiI2CSetup( AESD_I2C_ADDR );

    aesd_lcd_init( util_struct->i2c_fd );

    aesd_lcd_clear  ( util_struct->i2c_fd );
    aesd_lcd_loc    ( util_struct->i2c_fd, util_struct->line1 );
    aesd_lcd_type_ln( util_struct->i2c_fd, "In call_write_lcd_f" );
    aesd_lcd_loc    ( util_struct->i2c_fd, util_struct->line2 );
    aesd_lcd_type_ln( util_struct->i2c_fd, "Waiting for info" );

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

            value_pack( cnt_i, &ln2[8] );
            value_pack( pid_i, &ln1[9] );

            aesd_lcd_clear  ( util_struct->i2c_fd );
            aesd_lcd_loc    ( util_struct->i2c_fd, util_struct->line1 );
            aesd_lcd_type_ln( util_struct->i2c_fd, ln1 );
            aesd_lcd_loc    ( util_struct->i2c_fd, util_struct->line2 );
            aesd_lcd_type_ln( util_struct->i2c_fd, ln2 );

            util_struct->a[2] = 0;

            sleep(5);
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

    aesd_lcd_clear  ( util_struct->i2c_fd );
    aesd_lcd_loc    ( util_struct->i2c_fd, util_struct->line1 );
    aesd_lcd_type_ln( util_struct->i2c_fd, "Thanks but I'm" );
    aesd_lcd_loc    ( util_struct->i2c_fd, util_struct->line2 );
    aesd_lcd_type_ln( util_struct->i2c_fd, "out of here!!" );

    close(util_struct->i2c_fd);
    util_struct->i2c_fd = 0;

    return 0;
}

int test_func_1( struct aesd_struct *util_struct )
{
    int fd1 = 0;

    syslog( LOG_DEBUG, "Test func 1; testing LCD with ported functions" );

    if (wiringPiSetup () == -1) exit (1);

    fd1 = wiringPiI2CSetup( AESD_I2C_ADDR );

    // lcd_init(); // setup LCD
    aesd_lcd_init( fd1 );

    aesd_lcd_clear  ( fd1 );
    aesd_lcd_loc    ( fd1, util_struct->line1 );
    aesd_lcd_type_ln( fd1, "Thanks for the" );
    aesd_lcd_loc    ( fd1, util_struct->line2 );
    aesd_lcd_type_ln( fd1, "time we shared" );

    close(fd1);

    util_struct->i2c_fd = 0;

    return 0;
}