CXX = g++
CC = gcc

ifeq ($(CONFIG_DEBUG),y)
CFLAGS+=-g -O0 -Wall -DDEBUG
endif

CFLAGS += -Wall -I../include

%.o:%.c
	$(CC) $(CFLAGS) -c $<

%.o:%.cpp
	$(CXX) $(CFLAGS) -c $<

%.o:%.cxx
	$(CXX) $(CFLAGS) -c $<

libNxDb.a: blkio.o db.o nxdb.o index.o use.o
	$(AR) -ru $@ $^

blkio.o: blkio.c

db.o: db.c

nxdb.o: nxdb.cxx

index.o: index.c

use.o: use.c

clean:
	@ rm -f ide
	@ rm -f *.o
	@ rm -f *.a

install:

