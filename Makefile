# library makefile to create lib/timer/libas-timerEx.a

CC  ?=arm-none-linux-gnueabi-gcc
#CC ?=arm-linux-gnueabihf-gcc

IDIR   = -I
LIBDIR = lib/timer 

SRC1 = timerEx.c
STATLIB_TIMER=libas-timerEx.a
OBJ_TIMER = $(SRC1:.c=.o)

CFLAGS  += $(IDIR) -std=gnu99 -lrt -DGPS_LIB_BUILD -D_GNU_SOURCE

all: $(STATLIB_TIMER)

$(STATLIB_TIMER): $(OBJ_TIMER)
	ar rcs $@ $^
	sudo sh -c "cp libas-timerEx.a $(LIBDIR)"

.c.o:
	mkdir -p $(LIBDIR)
	@echo "[Compile c] - $(CC) $<"
	@$(CC) -c $(CFLAGS) $< -o $@
 
clean:
	rm -rf *.o
	rm -rf *.a

