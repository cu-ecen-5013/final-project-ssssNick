ifeq ($(CC),)
	CC   := $(CROSS_COMPILE)gcc
endif

ifeq ($(CFLAGS),)
	CFLAGS := -g -Wall -Werror
endif

ifeq ($(LDFLAGS),)
#	LDFLAGS := -pthread -lrt
endif

all : clean_all aesd_lcd_util

default : aesd_lcd_util

aesd_lcd_util : aesd_lcd_util.c
	$(CC) $(CFLAGS) -o aesd_lcd_util aesd_lcd_util.c $(INCLUDES) $(LDFLAGS)

clean :
	rm -f aesd_lcd_util
