# library makefile to create lib/libas-timerEx.a

# cross compile
CC ?=gcc
#CC ?=arm-none-linux-gnueabi-gcc
#CC ?=arm-linux-gnueabihf-gcc

IDIR   = -I
LIBDIR = lib

SRC1 = timerEx.c                  # file list
STATLIB_TIMER=libas-timerEx.a
OBJ_TIMER = $(SRC1:.c=.o)

CFLAGS  += $(IDIR) -std=gnu99 -lrt # -D_GNU_SOURCE

all: $(STATLIB_TIMER)

$(STATLIB_TIMER): $(OBJ_TIMER)
	ar rcs $@ $^
	sudo sh -c "mv libas-timerEx.a $(LIBDIR)"

.c.o:
	mkdir -p $(LIBDIR)
	@echo "[Compile c] - $(CC) $<"
	@$(CC) -c $(CFLAGS) $< -o $@
 
clean:
	rm -rf *.o
	rm -rf *.a

