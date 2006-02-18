OBJS = cache.o rfc1035.o dhcp.o dnsmasq.o
BINDIR = /usr/local/sbin
MANDIR = /usr/local/man

CFLAGS += -D__USE_BSD=1 -DNO_FORK=1
ifdef CONFIG_IPV6
CFLAGS += -DHAVE_IPV6
endif

all : dnsmasq

clean :
	rm -f *~ $(OBJS) dnsmasq dnsmasq.gdb

.c.o: dnsmasq.h config.h
	$(CC) -Wall -c $(CFLAGS) $*.c 


dnsmasq : $(OBJS) dnsmasq.h config.h
	$(CC) $(LDFLAGS) -o $@  $(OBJS) $(LDLIBS$(LDLIBS_$@))

install : dnsmasq
	install -d $(BINDIR) -d $(MANDIR)
	install -m 644 dnsmasq.8 $(MANDIR)/man8 
	install -m 755 dnsmasq $(BINDIR)

romfs:
	$(ROMFSINST) /bin/dnsmasq



