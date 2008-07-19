#CC=cc
#CFLAGS=-O -I../..

UNZ_OBJS = miniunz.o unzip.o ioapi.o $(ROOTDIR)/lib/libz.a
ZIP_OBJS = minizip.o zip.o   ioapi.o $(ROOTDIR)/lib/libz.a

.c.o:
	$(CC) -c $(CFLAGS) $*.c

all: miniunz minizip

miniunz:  $(UNZ_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(UNZ_OBJS)

minizip:  $(ZIP_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(ZIP_OBJS)

romfs:
	$(ROMFSINST) minizip /bin/zip
	$(ROMFSINST) miniunz /bin/unzip

test:	miniunz minizip
	./minizip test readme.txt
	./miniunz -l test.zip
	mv readme.txt readme.old
	./miniunz test.zip

clean:
	/bin/rm -f *.o *~ *.gdb minizip miniunz
