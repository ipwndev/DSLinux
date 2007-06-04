# Makefile for compiling the library test and hooks files.

CFLAGS		+= -O -Wunused -Wshadow -DUNIX -DLIBRARY

OBJECTS		= test.o lib.o

all: $(OBJECTS)

clean:
	rm -f *.o
