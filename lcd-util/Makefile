
EXE := aesd_lcd_util

OBJ := aesd_lcd_func.o aesd_lcd_util.o lcd.o wiringPi.o wiringPiI2C.o
DEP := aesd_lcd_util.h lcd.h version.h wiringPi.h wiringPiI2C.h

ifeq ($(CC),)
	CC   := $(CROSS_COMPILE)gcc
endif

ifeq ($(CFLAGS),)
	CFLAGS := -g -Wall -Werror
endif

ifeq ($(LDFLAGS),)
	LDFLAGS := -pthread -lrt
endif

all : clean $(EXE)

$(EXE) : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(INCLUDES) $(LDFLAGS)

%.o : %.c $(DEP)
	$(CC) $(CFLAGS) -c $< $(INCLUDES) $(LDFLAGS)


clean :
	rm -f *.o
	rm -f $(EXE)