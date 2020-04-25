
EXE := aesd_lcd_util_sp1

ifeq ($(CC),)
	CC   := $(CROSS_COMPILE)gcc
endif

ifeq ($(CFLAGS),)
	CFLAGS := -g -Wall -Werror
endif

ifeq ($(LDFLAGS),)
#	LDFLAGS := -pthread -lrt
endif

default : clean $(EXE)

all : clean_all subdir_lcd-util_all subdir_prj_base_all subdir_lcd_test_all $(EXE)

$(EXE) : aesd_lcd_util.c
	$(CC) $(CFLAGS) -o $(EXE) aesd_lcd_util.c $(INCLUDES) $(LDFLAGS)

subdir_lcd-util_all :
	$(MAKE) -C lcd-util all

subdir_prj_base_all :
	$(MAKE) -C prj_base all

subdir_lcd-test_all :
	$(MAKE) -C lcd-test all

clean :
	rm -f $(EXE)

clean_all : subdir_lcd-util_clean subdir_prj_base_clean subdir_lcd_test_clean
	rm -f $(EXE)

subdir_lcd-util_clean : 
	$(MAKE) -C lcd-util clean

subdir_prj_base_clean : 
	$(MAKE) -C prj_base clean

subdir_lcd-test_clean :
	$(MAKE) -C lcd-test clean