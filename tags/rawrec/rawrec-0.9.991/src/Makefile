# Copyright (C) 2006  Britton Leo Kerin, see copyright. 

PROG = rawrec
ALT_INVOC = rawplay

HEADERS = $(wildcard *.h)
OBJS = $(patsubst %.c,%.o,$(wildcard *.c))

CC = gcc
WARNFLAGS = -Wall -W -Wpointer-arith -Wstrict-prototypes -Wmissing-prototypes
DEBUGFLAGS = -O0 -g
OPTFLAGS = -O2

# Uncomment this to try to use untested priority ceiling/inheritance
# code (if system supports it).
#FANCY_THREAD_DEFINES = -DUSE_FANCY_PRIORITY_CODE

# FIXME: USEBUFFLOCK appears to be historic, and should probably go some day.
CFLAGS = $(WARNFLAGS) $(OPTFLAGS) $(DEBUGFLAGS) -DUSEBUFFLOCK \
         $(FANCY_THREAD_DEFINES) -D_REENTRANT -D_GNU_SOURCE \
         -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE
LDFLAGS = -lm -lpthread

# The GNU way is just too painful.  I'm not writing three tiers of
# variables just to get to /usr/local/man/man1.  If it starts to seem 
# worthwhile to use autoconf, this stuff can change then.
EXE_DIR = /usr/local/bin
MAN_DIR = /usr/local/man/man1

# If a build command exits with non-zero return code, delete any target 
# file corresponding to that command.  Probably not relevant for this 
# Makefile, but very much the right thing to do in general. 
.DELETE_ON_ERROR:

all: $(PROG) $(ALT_INVOC)

$(PROG): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $(PROG)

# Everything is rebuilt if this Makefile (which is hopefully named
# "Makefile") or any header changes.
$(OBJS): Makefile $(HEADERS)

$(ALT_INVOC): $(PROG)
	ln -sf $(PROG) $(ALT_INVOC)

# Build with debugging information.  As a bonus, emit extra warnings
# when compiling.  Needs gcc version 3.2 to be available by the name
# gcc-3.2 in order to work.  You may want to do a clean to force a
# rebuild of everything.
debug:
	$(MAKE) 'CC = gcc-3.2' \
		'DEBUGFLAGS = -ggdb3' \
		'OPTFLAGS = ' \
		'WARNFLAGS = $(WARNFLAGS) -Wbad-function-cast -Wcast-qual \
		-Wconversion -Wfloat-equal -Wsystem-headers' \
		all 

bin_install: all
	install -d $(EXE_DIR)
	install $(PROG) $(EXE_DIR)
	ln -sf $(PROG) $(EXE_DIR)/$(ALT_INVOC)
	chown root $(EXE_DIR)/$(PROG) $(EXE_DIR)/$(ALT_INVOC)
	chgrp root $(EXE_DIR)/$(PROG) $(EXE_DIR)/$(ALT_INVOC)
	@echo ''
	@echo '***** SECURITY WARNING *****'
	@echo ''
	@echo 'Making executable suid.'
	@echo '$(PROG) can provide soft real time scheduling capabilities as'
	@echo 'root that it cannot offer otherwise, but if security paranoia' 
	@echo 'is an issue, root permissions are no longer required.'
	@echo ''
	chmod go-w $(EXE_DIR)/$(PROG)
	chmod u+s $(EXE_DIR)/$(PROG)

man_install:
	install -d $(MAN_DIR)
	install --mode='u=rw,go=r' ../docs/user/$(PROG).1 $(MAN_DIR)
	ln -sf $(PROG).1 $(MAN_DIR)/$(ALT_INVOC).1

install: bin_install man_install

clean:
	rm -f $(PROG) $(ALT_INVOC) *.o

# This target is mainly used for development work.
distclean: clean
	@echo 'Cleaning src directory (cruft may remain in other dirs)...'
	rm -f TAGS
	rm -f *~

uninstall:
	rm -f $(EXE_DIR)/$(ALT_INVOC)
	rm -f $(EXE_DIR)/$(PROG)
	rm -f $(MAN_DIR)/$(ALT_INVOC).1
	rm -f $(MAN_DIR)/$(PROG).1

.PHONY: all debug install bin_install man_install clean distclean uninstall
