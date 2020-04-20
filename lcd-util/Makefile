
EXE := aesd_lcd_util

OBJ :=                          \
        aesd_lcd_func.o         \
        aesd_lcd_util.o         \
        aesd_write_lcd.o        \
        call_daemon_f.o         \
        call_read_test_f.o      \
        call_write_test_f.o     \
        rd_shared_mem_f.o       \
        setup_read_f.o          \
        struct_setup_f.o        \
        wiringPi.o              \
        wiringPiI2C.o

DEP :=                          \
        aesd_lcd_util.h         \
        version.h               \
        wiringPi.h              \
    	wiringPiI2C.h

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
