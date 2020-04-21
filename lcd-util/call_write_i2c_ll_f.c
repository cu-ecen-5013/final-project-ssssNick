
#include "aesd_lcd_util.h"

/******************************************************************************/
/**/
/******************************************************************************/
int call_write_i2c_ll_f( struct aesd_struct *util_struct )
{
    int ii = 0;
    int mx = 10;

    int cnt = 0;

    time_t cur = 0;
    time_t off = 30;

    struct aesd_ll_struct *nw_ptr = NULL;
    struct aesd_ll_struct *walker = NULL;
    struct aesd_ll_struct *shadow = NULL;

    syslog( LOG_DEBUG, "In CALL WRITE I2C LL function" ); 

    while( ii < mx )
    {
        nw_ptr = malloc( sizeof(struct aesd_ll_struct) );
        nw_ptr->count = ii;
        nw_ptr->ltime = time(NULL);
        nw_ptr->nxptr = NULL;

        walker = util_struct->head;

        if( walker == NULL )
        {
            util_struct->head = nw_ptr;
        }
        else
        {
            while( walker->nxptr != NULL )
            {
                walker = walker->nxptr;
            }
            walker->nxptr = nw_ptr;
        }

        cnt++;

        sleep(1);

        cur = time(NULL);
        
        walker = util_struct->head;

        while( walker != NULL )
        {
            if( cur > (walker->ltime + off) )
            {
                shadow = walker;
                walker = walker->nxptr;

                if( shadow == util_struct->head )
                {
                    util_struct->head = walker;
                }

                free( shadow );
                cnt--;
            }
            else
            {
                walker = walker->nxptr;
            }
        }


        sleep(1);

        syslog( LOG_DEBUG, "LL with head @ %p and relative count of %d", util_struct->head, cnt );

        walker = util_struct->head;

        while( walker != NULL )
        {
            syslog( LOG_DEBUG, "LL current ptr = %p with count %d and ltime %ld (with nxptr = %p)", walker, walker->count, walker->ltime, walker->nxptr );
            walker = walker->nxptr;
        }

        ii++;

        sleep(5);
    }

    while( util_struct->head != NULL )
    {
        walker = util_struct->head;
        util_struct->head = util_struct->head->nxptr;
        free(walker);
    }

    return 0;
}