# Makefile for zlib
# Copyright (C) 1995-1998 Jean-loup Gailly.
# For conditions of distribution and use, see copyright notice in zlib.h 

CFLAGS=-O3 -DHAVE_UNISTD_H -DUSE_MMAP $(ALLCFLAGS)

CPP=$(CC) -E

VER=1.1.3
LIBS=libz.a

AR=ar rc
RANLIB=ranlib

#OBJS = adler32.o compress.o crc32.o gzio.o uncompr.o trees.o \
#       zutil.o inflate.o infblock.o inftrees.o infcodes.o infutil.o inffast.o
OBJS = adler32.o zutil.o inflate.o infblock.o inftrees.o infcodes.o infutil.o inffast.o

libz.a: $(OBJS) $(OBJA)
	$(AR) $@ $(OBJS)
	-@ ($(RANLIB) $@ || true) >/dev/null 2>&1

clean:
	rm -f *.o libz.a

# DO NOT DELETE THIS LINE -- make depend depends on it.

adler32.o: zlib.h zconf.h
infblock.o: infblock.h inftrees.h infcodes.h infutil.h zutil.h zlib.h zconf.h
infcodes.o: zutil.h zlib.h zconf.h
infcodes.o: inftrees.h infblock.h infcodes.h infutil.h inffast.h
inffast.o: zutil.h zlib.h zconf.h inftrees.h
inffast.o: infblock.h infcodes.h infutil.h inffast.h
inflate.o: zutil.h zlib.h zconf.h infblock.h
inftrees.o: zutil.h zlib.h zconf.h inftrees.h
infutil.o: zutil.h zlib.h zconf.h infblock.h inftrees.h infcodes.h infutil.h
zutil.o: zutil.h zlib.h zconf.h  
