
#ifndef AESD_LCD_UTIL_H
#define AESD_LCD_UTIL_H

#include <ctype.h>
#include <errno.h>
#include <fcntl.h> 
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <syslog.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "wiringPi.h"
#include "wiringPiI2C.h"
#include "lcd.h"

#define ZERO  (0)
#define ONE   (1)
#define TWO   (2)
#define FIVE  (5)
#define K_1   (1000)

#define RD_VAL (2000)       /* ms  */
#define WR_VAL (30)         /* sec */

/////////////////////////////////////////////////////////////////

#define TEST_BUFFER_SIZE (85)

// Define some device parameters
#define I2C_ADDR   0x27 // I2C device address

// Define some device constants
#define LCD_CHR  1 // Mode - Sending data
#define LCD_CMD  0 // Mode - Sending command

#define LINE1  0x80 // 1st line
#define LINE2  0xC0 // 2nd line

#define LCD_BACKLIGHT   0x08  // On
// LCD_BACKLIGHT = 0x00  # Off

#define ENABLE  0b00000100 // Enable bit


int fd;

/////////////////////////////////////////////////////////////////

/* global exit flag; set by signal handler */
int *flag_to_exit;

/* structure to be passed around */
struct aesd_struct
{
    /* run flag */
    int flag_d;     /* daemon flag  */
    int flag_r;     /* read flag    */
    int flag_s;     /* seed flag    */
    int flag_t;     /* test flag    */
    int flag_w;     /* write flag   */

    /* exit flag set by sig handler */
    int flag_exit;

    int seed_val;

    /* pid returned from fork, if called */
    int pid_rtn;

    /* */
    int fd;

    int shmid;
    int status;

    int *a;
    int *b;

    int  pack_size;
    char pack_data[TEST_BUFFER_SIZE];

    /* function handlers */
    int (*daemon_f)( struct aesd_struct * );
    int (*read_f)  ( struct aesd_struct * );
    int (*write_f) ( struct aesd_struct * );
};

int call_daemon_f    ( struct aesd_struct * );
int call_default_f   ( struct aesd_struct * );
int call_read_lcd_f  ( struct aesd_struct * );
int call_write_lcd_f ( struct aesd_struct * );
int call_write_test_f( struct aesd_struct * );

int setup_read_f     ( struct aesd_struct * );
int call_read_test_f ( struct aesd_struct * );
int rd_shared_mem_f  ( struct aesd_struct * );


int write_lcd_f      ( struct aesd_struct * );

/* close and clean-up anything needed before exiting */
void clean_up_f           ( struct aesd_struct *util_struct );

/* common signal handler */
void common_sig_handle_f  ( int sig );


/* set default values for flags */
/* connect global exit flag */
/* parse input parameters */
/* setup function pointers */
int struct_setup_f ( int , char **, struct aesd_struct * );


void value_pack   ( int input_val, char *buf );
int  value_unpack ( char *buf );

/////////////////////////////////////////////////////////////////

void lcd_init(void);
void lcd_byte(int bits, int mode);
void lcd_toggle_enable(int bits);

// added by Lewis
void typeInt(int i);
void typeFloat(float myFloat);
void lcdLoc(int line); //move cursor
void ClrLcd(void); // clr LCD return home
void typeln(const char *s);
void typeChar(char val);

/////////////////////////////////////////////////////////////////

#endif