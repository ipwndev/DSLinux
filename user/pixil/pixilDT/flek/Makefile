# pixilDT/flek/Makefile

# Indicate that we want to go native

BUILD_NATIVE=y
TARGET_EXTRAS=libflek.a

CPPFILES=${shell ls $(FLEKDIR)/src/*.cxx}
OBJS = $(subst $(FLEKDIR)/src/,, $(CPPFILES:.cxx=_native.o))

INCLUDES=-I. -I../

include $(BASE_DIR)/Rules.make

libflek.a: $(OBJS)
	$(AR) rc $@ $(OBJS)
	$(RANLIB) $@

%_native.o: $(FLEKDIR)/src/%.cxx
	$(CXX) -c $(CFLAGS) $(INCLUDES) -o $@ $<
