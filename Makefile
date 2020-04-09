ifeq ($(CC),)
	CC   := $(CROSS_COMPILE)gcc
endif

ifeq ($(CFLAGS),)
	CFLAGS := -g -Wall -Werror
endif

ifeq ($(LDFLAGS),)
	LDFLAGS := -pthread -lrt
endif

OBJ := aesd_lcd_read.o aesd_lcd_write.o aesd_lcd_func.o aesd_lcd_util.o

DEP := prj_base/aesd_lcd_util.h

all : clean_all aesd_lcd_util

default : aesd_lcd_util

aesd_lcd_util : $(OBJ)
	$(CC) $(CFLAGS) -o aesd_lcd_util $^ $(INCLUDES) $(LDFLAGS)

%.o : prj_base/%.c $(DEP)
	$(CC) $(CFLAGS) -c $^ $(INCLUDES) $(LDFLAGS)

clean :
	rm -f *.o

clean_all :
	rm -f *.o
	rm -f prj_base/aesd_lcd_util.h.gch
	rm -f aesd_lcd_util
