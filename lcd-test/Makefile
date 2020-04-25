# Author: Nick Brubaker

ifeq ($(CC),)
	CC:=$(CROSS_COMPILE)gcc
endif

ifeq ($(CFLAGS),)
	CFLAGS = -g -Wall -Werror
endif

ifeq ($(LDFLAGS),)
	LDFLAGS =
endif

default: all

all: lcd_test

lcd_test: lcd_test.c
	$(CC) $(CFLAGS) -o "lcd_test" "lcd_test.c" $(LDFLAGS)

clean:
	-rm -f *.o lcd_test
