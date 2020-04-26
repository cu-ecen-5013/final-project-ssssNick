
#include "aesd_lcd_util.h"

/******************************************************************************/
/* re-purposed functions from the lcd example                                 */
/* in wiringPi                                                                */
/******************************************************************************/

/******************************************************************************/
/**/
/******************************************************************************/
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

/******************************************************************************/
/**/
/******************************************************************************/
void aesd_lcd_clear( int input_fd )
{
    aesd_lcd_byte( input_fd, 0x01, AESD_LCD_CMD );
    aesd_lcd_byte( input_fd, 0x02, AESD_LCD_CMD );
}

/******************************************************************************/
/**/
/******************************************************************************/
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

/******************************************************************************/
/**/
/******************************************************************************/
void aesd_lcd_loc( int input_fd, int line )
{
    aesd_lcd_byte( input_fd, line, AESD_LCD_CMD );
}

/******************************************************************************/
/**/
/******************************************************************************/
void aesd_lcd_toggle( int input_fd, int bits )
{
    // Toggle enable pin on LCD display
    delayMicroseconds(500);
    wiringPiI2CReadReg8(input_fd, (bits | AESD_LCD_ENABLE));
    delayMicroseconds(500);
    wiringPiI2CReadReg8(input_fd, (bits & ~AESD_LCD_ENABLE));
    delayMicroseconds(500);
}

/******************************************************************************/
/**/
/******************************************************************************/
void aesd_lcd_type_ln( int input_fd, const char *s )
{
    while( *s )
    {
        aesd_lcd_byte( input_fd, *(s++), AESD_LCD_CHR);
    }
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
int call_read_lcd_f( struct aesd_struct *util_struct )
{
    return 0;
}

/******************************************************************************/
/**/
/******************************************************************************/
void clean_up_f( struct aesd_struct *util_struct )
{
    struct aesd_ll_struct *walker = NULL;

    syslog( LOG_DEBUG, "Cleaning up aesd util" );

    while( util_struct->head != NULL )
    {
        walker = util_struct->head;
        util_struct->head = util_struct->head->nxptr;
        free(walker);

        syslog( LOG_DEBUG, "Freeing ll memory" );
    }

    if( util_struct->lcd_fd > 0 )
    {
        syslog( LOG_DEBUG, "Closing LCD FD" ); 
        close( util_struct->lcd_fd );
        util_struct->lcd_fd = 0;
    }

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