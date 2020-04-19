
#include "aesd_lcd_util.h"

int call_write_lcd_f ( struct aesd_struct *util_struct )
{
    int cnt_i;
    int pid_i;

    char ln1 [16] = "From PID _____";
    char ln2 [16] = "Rec val _____";

    if (wiringPiSetup () == -1) exit (1);

    fd = wiringPiI2CSetup(I2C_ADDR);

    lcd_init(); // setup LCD

    syslog( LOG_DEBUG, "Func 2 screen with packet @%p", util_struct );

    ClrLcd();
    lcdLoc( LINE1 );
    typeln( "In call_write_lcd_f"   );
    lcdLoc( LINE2 );
    typeln( "Waiting for info"   );

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

            ClrLcd();
            lcdLoc( LINE1 );
            typeln( ln1   );
            lcdLoc( LINE2 );
            typeln( ln2   );

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

    ClrLcd();
    lcdLoc( LINE1 );
    typeln( "Thanks but I'm"   );
    lcdLoc( LINE2 );
    typeln( "out of here!!"   );

    close(fd);

    return 0;
}