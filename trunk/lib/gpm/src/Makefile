# Generated automatically from Makefile.in by configure.
#			-*-makefile-*- (gpm/src)
#
# Copyright 1994,1997  rubini@linux.it
# Copyright 1997       dickey@clark.net
# Copyright (C) 1998 Ian Zimmerman <itz@rahul.net>
# Copyright (C) 2001,2002 Nico Schottelius <nico@schottelius.org>
#

srcdir = $(ROOTDIR)/lib/gpm/src
top_builddir = ..

include $(top_builddir)/Makefile.include

# Main portion: regular build rules

GSRC = main.c gpm.c gpn.c mice.c special.c twiddler.c synaptics.c \
       startup.c server_tools.c

GOBJ = $(GSRC:.c=.o) report.o tools.o

LSRC = lib/liblow.c lib/libhigh.c lib/libxtra.c lib/report-lib.c

LOBJ = $(LSRC:.c=.o) tools.o

PICS = $(LOBJ:.o=.lo)

HDRS = gpm.h gpmInt.h twiddler.h synaptics.h message.h

PSRC = prog/mev.c prog/hltest.c prog/mouse-test.c prog/disable-paste.c

POBJ = $(PSRC:.c=.o) prog/gpm-root.o

PROG = $(POBJ:.o=)

SRCS = $(GSRC) $(LSRC) $(PSRC) report.c

DEFS  = -DHAVE_CONFIG_H

STRIP = -s

LIBS += -lm

### BUILD PROCESS

# allow CFLAGS to be overriden from make command line
# ideally one would never have to write this rule again, but the GNU
# makefile standards are at cross-purposes: CFLAGS is reserved for
# user-overridable flags, but it's also all the implicit rule looks at.

# the prog rules are not very clean...
prog/%.o: prog/%.c
	$(CC) -I$(ROOTDIR)/lib/libncurses/include -I$(srcdir) $(DEFS) -include headers/config.h -Wall -DSYSCONFDIR="\"$(sysconfdir)\"" -DSBINDIR="\"$(sbindir)\"" $(CPPFLAGS) -g -O2 $(CFLAGS) -c -o $@ $<

prog/%:	prog/%.o
	$(CC) -L$(srcdir) $(LDFLAGS) -o $@ $^  $(LIBS)

%.o: %.c
	$(CC) -I$(ROOTDIR)/lib/libncurses/include -I$(srcdir) $(DEFS) -include headers/config.h -Wall -DSYSCONFDIR="\"$(sysconfdir)\"" -DSBINDIR="\"$(sbindir)\"" $(CPPFLAGS) -g -O2 $(CFLAGS) -c -o $@ $<

%.lo:	%.c
	$(CC) -I$(ROOTDIR)/lib/libncurses/include -I$(srcdir) $(DEFS) -include headers/config.h -Wall -DSYSCONFDIR="\"$(sysconfdir)\"" -DSBINDIR="\"$(sbindir)\"" $(CPPFLAGS) -DPIC -fPIC -g -O2 $(CFLAGS) -c -o $@ $<

%:	%.o
	$(CC) -L$(srcdir) $(LDFLAGS) -o $@ $^  $(LIBS)

# old, unused, but good rule [dependings]
#%.d: $(srcdir)/%.c
#	$(SHELL) -ec '$(CC) -M $(GPMCPPFLAGS) $(CPPFLAGS) $< \
#		| $(SED) '\''s/\($*\)\.o\([ :]*\)/\1.o \1.lo\2/g'\'' > $(DEPDIR)/$@'

# Do it all!
all:	gpm lib/libgpm.a #lib/libgpm.so  $(PROG)

gpm:	$(GOBJ)

# construct dependings of sourcefiles and link sourcefiles
$(DEPFILE) dep: prog/gpm-root.c
   # make links in subdirectories
	-@for cfiledirs in prog lib; do $(MKDIR) $$cfiledirs; \
	cd $$cfiledirs; for cfiles in `echo $(srcdir)/$$cfiledirs/*.c`; \
	do $(LN_S) $$cfiles 2>/dev/null; done; cd ..; done

	# make links in srcdir
	-@for cfiles in `echo $(srcdir)/*.c`; do $(LN_S) $$cfiles 2>/dev/null; done

	# create dependencies
	for DEPS in `echo *.c */*.c`; do \
	$(CC)  -I$(ROOTDIR)/lib/libncurses/include -I. -I $(srcdir) -M -I$(srcdir) $(DEFS) -include headers/config.h -Wall -DSYSCONFDIR="\"$(sysconfdir)\"" -DSBINDIR="\"$(sbindir)\"" $(CPPFLAGS) $$DEPS | \
	$(SED) 's/\($*\)\.o\([ :]*\)/\1.o \1.lo\2/g'  >> $(DEPFILE) ; done

### INSTALL
check:	all
installdirs:

install:	check 
	$(INSTALL_PROGRAM) gpm $(sbindir)/gpm
	$(INSTALL_DATA) -m 644 lib/libgpm.a $(libdir)/libgpm.a
	$(INSTALL_DATA) -m 644 $(srcdir)/headers/gpm.h $(includedir)/gpm.h
	# POSIX requires the range of a for loop be nonempty and Bash
	# 2.x goes along; unfortunately that means an additional
	# headache in cases like this
	if test "xlibgpm.so" != "x" ; then \
		$(INSTALL_DATA) -m 644 lib/libgpm.so.1.19.0 $(libdir)/libgpm.so.1.19.0 ;\
		cd $(libdir) && $(LN_S) -f libgpm.so.1.19.0 libgpm.so ;\
      echo "WARNING: We installed a lib, you should now call ldconfig" ; \
      echo "f.i.: ldconfig -n -l $(libdir)/libgpm.so.1.19.0" ;\
      echo "Or to update everything just type ldconfig"; \
	fi

   # prog/
	for i in $(PROG); do \
		$(INSTALL_PROGRAM) $$i $(bindir)/`echo $$i | sed 's,prog/,,'` ;\
	done

install-strip:
	$(MAKE) INSTALL_PROGRAM='$(INSTALL_PROGRAM) $(STRIP)' install

# Note: this leaves around /usr/lib/libgpm.so.1 and /usr/lib/libgpm.so.1.nn
# This is probably the right thing, because programs may be linked to it
uninstall:
	rm -f $(bindir)/mev $(bindir)/gpm-root $(bindir)/disable-paste \
	$(sbindir)/gpm $(libdir)/libgpm.a $(libdir)/libgpm.so $(includedir)/gpm.h 
	for i in $(PROG); do \
		rm -f $(bindir)/$$i ;\
	done

# cleaning up section

clean:
	rm -f gpm lib/libgpm.a lib/libgpm.so lib/libgpm.so.1.19.0 $(RDEPS)
	rm -f core *~ $(GOBJ) $(LOBJ) $(POBJ) $(PICS) gpm-root.c $(DEPFILE)
	rm -f $(PROG) $(POBJ) prog/gpm-root.c

distclean: clean
	rm -f headers/config.h headers/config.h.in Makefile $(DEPFILE)

# Configure & unconfigure

# Subdirectory prog/
prog/gpm-root.c:	$(srcdir)/prog/gpm-root.y
	$(YACC) $(YFLAGS) $< && mv y.tab.c prog/gpm-root.c

# gpm-root needs an own rule, because gpm-root.c is not in $(srcdir)
prog/gpm-root: prog/gpm-root.c
	$(CC) -I$(ROOTDIR)/lib/libncurses/include -I. -I$(srcdir) $(DEFS) -include headers/config.h -Wall -DSYSCONFDIR="\"$(sysconfdir)\"" -DSBINDIR="\"$(sbindir)\"" $(CPPFLAGS) -g -O2 $(CFLAGS) -c -o $@.o $<
	$(CC) -L$(srcdir) $(LDFLAGS) -o $@ $@.o  $(LIBS) lib/libgpm.a

prog/mouse-test:	mice.o twiddler.o synaptics.o

$(PROG):	lib/libgpm.so lib/libgpm.so lib/libgpm.a

# Subdirectory lib/
lib/libgpm.a:	$(LOBJ)
	$(AR) rc lib/libgpm.a $(LOBJ)
	$(RANLIB) lib/libgpm.a

lib/libgpm.so.1.19.0:	$(PICS)
	$(CC) -fPIC -shared -Wl,-soname,libgpm.so.1 \
	-L$(srcdir) $(LDFLAGS) -o lib/libgpm.so.1.19.0 $^  -L$(ROOTDIR)/lib/libncurses/lib -lncurses  -lncurses  -lc $(LIBS)	
lib/libgpm.so:	lib/libgpm.so.1.19.0
	$(LN_S) -f libgpm.so.1.19.0 lib/libgpm.so

include $(DEPFILE)

