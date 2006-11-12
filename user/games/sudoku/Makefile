#!/usr/bin/make

# Makefile for DSLinux by Cayenne Boyer (cayennes@gmail.com)

EXEC=sudoku
OBJS=sudoku.o
LIBS =  -L../../../lib/libncurses/lib -lncurses
CFLAGS += -I../../../lib/libncurses/include

.PHONY: all romfs clean

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

romfs: sudoku
	$(ROMFSINST) /usr/games/$(EXEC)
	$(ROMFSINST) -d template /usr/share/games/sudoku/template
	
clean:
	rm -f $(EXEC) *.elf *.gdb *.o
