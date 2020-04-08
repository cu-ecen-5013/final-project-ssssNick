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
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define FALSE (0)
#define TRUE  (1)

#define TEST_BUFFER_SIZE (25)

/* global exit flag; set by signal handler */
int *flag_to_exit;

/* structure to be passed around */
struct aesd_struct
{
    /* run flag */
    int flag_d;     /* daemon flag  */
    int flag_r;     /* read flag    */
    int flag_t;     /* test flag    */
    int flag_w;     /* write flag   */

    /* exit flag set by sig handler */
    int flag_exit;

    /* pid returned from fork, if called */
    int pid_rtn;

    /* */
    int fd;

    /* function handlers */
    int (*daemon_f)( struct aesd_struct * );
    int (*read_f)  ( struct aesd_struct * );
    int (*write_f) ( struct aesd_struct * );
};

int call_daemon_f    ( struct aesd_struct * );
int call_default_f   ( struct aesd_struct * );
int call_read_lcd_f  ( struct aesd_struct * );
int call_read_test_f ( struct aesd_struct * );
int call_write_lcd_f ( struct aesd_struct * );
int call_write_test_f( struct aesd_struct * );

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


#endif