# Makefile for mpc part 2
#
# Originally based from hexedit Makefile
#
# (by Tuna)

INCL = charConv.h libmpdclient.h mpc.h password.h util.h command.h list.h options.h status.h ../config.h
SRCS = charConv.c libmpdclient.c  main.c password.c util.c command.c list.c options.c status.c

OBJS = $(SRCS:.c=.o)

PRODUCT = mpc

.SUFFIXES: .c .o

.c.o:
	$(CC) $(DEFS) $(CFLAGS) -c $<

all: $(PRODUCT)

$(PRODUCT): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

romfs:
	$(ROMFSINST) /bin/$(PRODUCT)

clean:
	rm -rf *~ *.o core *.cache config.status config.log $(PRODUCT) $(PRODUCT).gdb

#distclean: clean
#	rm -f Makefile config.h

install: $(PRODUCT)
	$(INSTALL) -d -m 755 $(bindir)
	$(INSTALL) -s -m 755 $(PRODUCT) $(bindir)
	$(INSTALL) -d -m 755 $(mandir)/man1
	$(INSTALL) -m 644 $(PRODUCT).1 $(mandir)/man1
