# pixilDT/fltk/Makefile

# Indicate that we want to go native
BUILD_NATIVE=y

TARGET_EXTRAS=libfltk.a

CPPFILES=${shell ls $(FLTKDIR)/src/*.cxx}
CFILES=${shell ls $(FLTKDIR)/src/*.c}

OBJS = $(subst $(FLTKDIR)/src/,, $(CPPFILES:.cxx=_native.o))
OBJS+= $(subst $(FLTKDIR)/src/,, $(CFILES:.c=_native.o))

INCLUDES=-I. -I../ -I$(FLTKDIR)/src

include $(BASE_DIR)/Rules.make

libfltk.a: $(OBJS)
	$(AR) rc $@ $(OBJS)
	$(RANLIB) $@

%_native.o: $(FLTKDIR)/src/%.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<

%_native.o: $(FLTKDIR)/src/%.cxx
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
