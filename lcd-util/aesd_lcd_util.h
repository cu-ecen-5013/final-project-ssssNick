
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

#define ZERO  (0)
#define ONE   (1)
#define TWO   (2)
#define FIVE  (5)
#define K_1   (1000)

#define RD_VAL (2000)       /* ms  */
#define WR_VAL (30)         /* sec */

#define TEST_BUFFER_SIZE (85)

/********************************************/
/* re-purposed defines from the lcd example */
/* in wiringPi                              */
/********************************************/
#define AESD_I2C_ADDR   (0x27)
#define AESD_LCD_CHR    (1) // Mode - Sending data
#define AESD_LCD_CMD    (0) // Mode - Sending command
#define ASED_16x2_LINE1 (0x80) // 1st line
#define ASED_16x2_LINE2 (0xC0) // 2nd line
#define AESD_LCD_BACKLT (0x08)  // On
#define AESD_LCD_ENABLE (0b00000100) // Enable bit
//////////////////////////////////////////////

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

    int i2c_fd;

    int shmid;
    int status;

    int *a;
    int *b;

    int  pack_size;
    char pack_data[TEST_BUFFER_SIZE];

    int line1;
    int line2;
    int line3;
    int line4;

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

int test_func_1( struct aesd_struct *util_struct );

/**********************************************/
/* re-purposed functions from the lcd example */
/* in wiringPi                                */
/**********************************************/
void aesd_lcd_init   ( int input_fd );
void aesd_lcd_byte   ( int input_fd, int bits, int mode );
void aesd_lcd_clear  ( int input_fd );
void aesd_lcd_loc    ( int input_fd, int line );
void aesd_lcd_type_ln( int input_fd, const char *s );
void aesd_lcd_toggle ( int input_fd, int bits );

#endif