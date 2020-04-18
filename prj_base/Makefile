EXE := aesd_lcd_util_org

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

DEP := aesd_lcd_util.h

all : clean_all $(EXE)

default : $(EXE)

$(EXE) : $(OBJ)
	$(CC) $(CFLAGS) -o $(EXE) $^ $(INCLUDES) $(LDFLAGS)

%.o : %.c $(DEP)
	$(CC) $(CFLAGS) -c $< $(INCLUDES) $(LDFLAGS)

clean :
	rm -f *.o
	rm -f $(EXE) 

clean_all :
	rm -f *.o
	rm -f $(EXE).h.gch
	rm -f $(EXE) 