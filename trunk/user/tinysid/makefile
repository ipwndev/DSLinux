# TinySID makefile
DEFS = -Wall
CFLAGS = $(DEFS) -O
LDFLAGS =
LIBS = -lpthread
CC = gcc

OBJECTS_A = tinysid.o sidengine.o soundcard.o

ARCH_FILES = tinysid.c sidengine.c soundcard.c makefile defines.h

tinysid: $(OBJECTS_A)
		$(CC) -o tinysid $(LDFLAGS) $(OBJECTS_A) $(LIBS)

sidengine.o: sidengine.c
		$(CC) -c $(CFLAGS) sidengine.c

soundcard.o: soundcard.c
		$(CC) -c $(CFLAGS) soundcard.c
		
clean:
	echo Cleaning up...
	rm *.o
	echo OK.
